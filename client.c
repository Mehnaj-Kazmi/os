#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

GtkWidget *window;
GtkWidget *text_view;
GtkWidget *entry;
GtkWidget *send_button;

int socketDescriptor;

void send_message(GtkWidget *widget, gpointer data) {
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry));
    send(socketDescriptor, message, strlen(message), 0);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

gboolean update_message_display(gpointer data) {
    char *message = (char *)data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, message, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    g_free(message);
    return FALSE;
}

void receive_message() {
    char recvBuffer[1000];
    while(1) {
        bzero(&recvBuffer,sizeof(recvBuffer));
        recv(socketDescriptor, recvBuffer, sizeof(recvBuffer), 0);
        if (strlen(recvBuffer) > 0) {
            g_idle_add((GSourceFunc)update_message_display, g_strdup(recvBuffer)); // Update GUI from main thread
        }
    }
}

int main() {
    struct sockaddr_in serverAddress;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);

    send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, TRUE, 0);

    gtk_widget_show_all(window);

    bzero(&serverAddress,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(5500);

    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    connect(socketDescriptor,(struct sockaddr*)&serverAddress,sizeof(serverAddress));

    g_thread_new("receiver", (GThreadFunc)receive_message, NULL); // Start receiving messages in a separate thread

    gtk_main();

    close(socketDescriptor);
    return 0;
}
