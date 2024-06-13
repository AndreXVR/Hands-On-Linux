#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/string.h>
// #include <linux/stdio.h>
// #include <linux/stdlib.h>
#include <linux/slab.h> // Para kmalloc e kfree


MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102");
MODULE_LICENSE("GPL");


#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositvo USB


static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saida da USB
static char *usb_in_buffer, *usb_out_buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB

#define VENDOR_ID   0x10c4 /* Encontre o VendorID  do smartlamp */
#define PRODUCT_ID  0xea60 /* Encontre o ProductID do smartlamp */
static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); // Executado quando o dispositivo é conectado na USB
static void usb_disconnect(struct usb_interface *ifce);                           // Executado quando o dispositivo USB é desconectado da USB
static int  usb_read_serial(void);                                                   // Executado para ler a saida da porta serial

MODULE_DEVICE_TABLE(usb, id_table);
bool ignore = true;
int LDR_value = 0;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     // Nome do driver
    .probe       = usb_probe,       // Executado quando o dispositivo é conectado na USB
    .disconnect  = usb_disconnect,  // Executado quando o dispositivo é desconectado na USB
    .id_table    = id_table,        // Tabela com o VendorID e ProductID do dispositivo
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);  // AQUI
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);

    LDR_value = usb_read_serial();

    printk("LDR Value: %d\n", LDR_value);

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    kfree(usb_in_buffer);                   // Desaloca buffers
    kfree(usb_out_buffer);
}

static int usb_read_serial() {
    int ret, actual_size;
    int retries = 10;
    // char message[];           // Tenta algumas vezes receber uma resposta da USB. Depois desiste.
    // // char *message = malloc(1);
    // message[0] = '\0';
    char *message = NULL; // Inicializa a string como nula
    int message_size = 0; //

    // Espera pela resposta correta do dispositivo (desiste depois de várias tentativas)
    while (retries > 0) {

        // Lê os dados da porta serial e armazena em usb_in_buffer
        // usb_in_buffer - contem a resposta em string do dispositivo
        // actual_size - contem o tamanho da resposta em bytes
        ret = usb_bulk_msg(smartlamp_device, usb_rcvbulkpipe(smartlamp_device, usb_in), usb_in_buffer, min(usb_max_size, MAX_RECV_LINE), &actual_size, 1000);
        if (ret) {
            printk(KERN_ERR "SmartLamp: Erro ao ler dados da USB (tentativa %d). Codigo: %d\n", ret, retries--);
            continue;
        }

        printk(KERN_ERR "USB IN BUFFER: %s\n", usb_in_buffer);
        printk(KERN_ERR "MESSAGE: %s\n", message);

        // Aloca memória para a nova string com base no tamanho atual da mensagem e do tamanho da resposta
        char *temp = kmalloc(message_size + actual_size + 1, GFP_KERNEL);
        // if (!temp) {
        //     printk(KERN_ERR "SmartLamp: Erro ao alocar memória para a nova mensagem.\n");
        //     return NULL;
        // }

        memcpy(temp, message, message_size);
        // Copia os caracteres recebidos da USB para a nova string
        memcpy(temp + message_size, usb_in_buffer, actual_size);
        // Atualiza o tamanho da mensagem
        message_size += actual_size;

        // Adiciona o caractere nulo ao final da nova string
        temp[message_size] = '\0';

        // Libera a memória da mensagem anterior
        kfree(message);
        // Atribui a nova string à mensagem
        message = temp;

        if (strchr(usb_in_buffer, '\n')) {
            //caso tenha recebido a mensagem 'RES_LDR X' via serial acesse o buffer 'usb_in_buffer' e retorne apenas o valor da resposta X
            //retorne o valor de X em inteiro

            // Procura por 'RES_LDR ' no início da resposta
            char *start = strstr(message, "RES GET_LDR "); // retorna a primeira ocorrência da "RES_LDR " na string usb_in_buffer
            // usb_in_buffer[actual_size] = '\0';
            if (!start) {
                printk(KERN_ERR "SmartLamp: Mensagem incompleta ou inválida.\n");
                continue;
            }

            // Extrai o valor de X após 'RES_LDR'
            start += strlen("RES GET_LDR "); // Move o ponteiro para o início do valor de X
            // int value = atoi(start);
            int value;
            sscanf(start, "%d", &value);
            printk(KERN_ERR "COMMAND DETECTED. VALUE: %d\n", value);
            return value;
        }
    }

    return -1; 
}