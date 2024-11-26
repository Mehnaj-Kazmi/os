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

int clientSocketDescriptor, socketDescriptor;

void *receive_message() {
    char recvBuffer[1000];
    while (1) {
        bzero(&recvBuffer,sizeof(recvBuffer));
        recv(clientSocketDescriptor, recvBuffer, sizeof(recvBuffer), 0);
        if (strlen(recvBuffer) > 0) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, recvBuffer, -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
    }
    return NULL;
}

void send_message(GtkWidget *widget, gpointer data) {
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry));
    send(clientSocketDescriptor, message, strlen(message), 0);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clientLength;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Server");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);

    GtkWidget *send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, TRUE, 0);

    gtk_widget_show_all(window);

    bzero(&serverAddress,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(5500);

    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    bind(socketDescriptor,(struct sockaddr*)&serverAddress,sizeof(serverAddress));
    listen(socketDescriptor,5);
    printf("%s\n","Server is running ...");

    clientSocketDescriptor = accept(socketDescriptor,(struct sockaddr*)&clientAddress,&clientLength);

    pthread_t tid;
    pthread_create(&tid, NULL, receive_message, NULL);

    gtk_main();

    close(socketDescriptor);
    return 0;
}
