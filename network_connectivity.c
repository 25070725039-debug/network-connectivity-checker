#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- Data structures ---------------- */

typedef struct Node {
    int id;
    struct Node *next;
} Node;

typedef struct {
    int n;         /* number of computers */
    int cables;    /* number of cables (edges) */
    Node **adj;    /* adjacency list */
} Network;

/* ---------------- Input helper ---------------- */

/* Reads one integer safely. Returns 0 only on end of input. */
int read_int(const char *prompt, int *out) {
    char line[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) return 0;
        if (sscanf(line, "%d", out) == 1) return 1;
        printf("  Invalid input. Please enter a number.\n");
    }
}

/* ---------------- Graph operations ---------------- */

Network *create_network(int n) {
    Network *net = malloc(sizeof(Network));
    net->n = n;
    net->cables = 0;
    net->adj = calloc(n, sizeof(Node *));
    return net;
}

int has_cable(Network *net, int a, int b) {
    for (Node *p = net->adj[a]; p; p = p->next)
        if (p->id == b) return 1;
    return 0;
}

/* Append neighbour at the END so traversal follows insertion order */
static void add_one_way(Network *net, int from, int to) {
    Node *node = malloc(sizeof(Node));
    node->id = to;
    node->next = NULL;
    if (net->adj[from] == NULL) {
        net->adj[from] = node;
    } else {
        Node *p = net->adj[from];
        while (p->next) p = p->next;
        p->next = node;
    }
}

/* Returns 1 on success, 0 if the cable was rejected */
int add_connection(Network *net, int a, int b) {
    if (a < 0 || a >= net->n || b < 0 || b >= net->n) {
        printf("  Error: computer numbers must be between 0 and %d.\n", net->n - 1);
        return 0;
    }
    if (a == b) {
        printf("  Error: a computer cannot be cabled to itself.\n");
        return 0;
    }
    if (has_cable(net, a, b)) {
        printf("  That cable already exists.\n");
        return 0;
    }
    add_one_way(net, a, b);
    add_one_way(net, b, a);
    net->cables++;
    return 1;
}

void free_network(Network *net) {
    for (int i = 0; i < net->n; i++) {
        Node *cur = net->adj[i];
        while (cur) {
            Node *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(net->adj);
    free(net);
}

/* ---------------- Traversals ---------------- */
/* Each traversal marks visited[], records visiting order in order[],
   and returns the number of computers reached. */

int bfs(Network *net, int start, int visited[], int order[]) {
    int *queue = malloc(net->n * sizeof(int));
    int front = 0, rear = 0, count = 0;

    visited[start] = 1;
    queue[rear++] = start;

    while (front < rear) {
        int u = queue[front++];
        order[count++] = u;
        for (Node *p = net->adj[u]; p; p = p->next) {
            if (!visited[p->id]) {
                visited[p->id] = 1;
                queue[rear++] = p->id;
            }
        }
    }
    free(queue);
    return count;
}

/* Recursive helper for DFS */
static void dfs_visit(Network *net, int u, int visited[], int order[], int *count) {
    visited[u] = 1;
    order[(*count)++] = u;
    for (Node *p = net->adj[u]; p; p = p->next)
        if (!visited[p->id])
            dfs_visit(net, p->id, visited, order, count);
}

int dfs(Network *net, int start, int visited[], int order[]) {
    int count = 0;
    dfs_visit(net, start, visited, order, &count);
    return count;
}

/* ---------------- Menu actions ---------------- */

void print_order(const char *label, int order[], int count) {
    printf("  %s order: ", label);
    for (int i = 0; i < count; i++)
        printf("%d%s", order[i], i < count - 1 ? " -> " : "\n");
}

void check_connectivity(Network *net, int use_bfs) {
    int *visited = calloc(net->n, sizeof(int));
    int *order = malloc(net->n * sizeof(int));

    int reached = use_bfs ? bfs(net, 0, visited, order)
                          : dfs(net, 0, visited, order);

    print_order(use_bfs ? "BFS" : "DFS", order, reached);
    printf("  Reached %d of %d computers.\n", reached, net->n);
    if (reached == net->n)
        printf("  RESULT: The network is CONNECTED.\n");
    else
        printf("  RESULT: The network is NOT connected.\n");

    free(visited);
    free(order);
}

void show_unreachable(Network *net) {
    int *visited = calloc(net->n, sizeof(int));
    int *order = malloc(net->n * sizeof(int));
    bfs(net, 0, visited, order);

    int found = 0;
    printf("  Computers unreachable from computer 0:");
    for (int i = 0; i < net->n; i++) {
        if (!visited[i]) {
            printf(" %d", i);
            found = 1;
        }
    }
    printf(found ? "\n" : " none\n");

    free(visited);
    free(order);
}

void count_components(Network *net) {
    int *visited = calloc(net->n, sizeof(int));
    int *order = malloc(net->n * sizeof(int));
    int components = 0;

    for (int i = 0; i < net->n; i++) {
        if (!visited[i]) {
            int size = bfs(net, i, visited, order);
            components++;
            printf("  Group %d:", components);
            for (int j = 0; j < size; j++) printf(" %d", order[j]);
            printf("\n");
        }
    }
    printf("  Total separate groups: %d\n", components);

    free(visited);
    free(order);
}

void check_path(Network *net) {
    int a, b;
    if (!read_int("  From computer: ", &a)) return;
    if (!read_int("  To computer:   ", &b)) return;
    if (a < 0 || a >= net->n || b < 0 || b >= net->n) {
        printf("  Error: computer numbers must be between 0 and %d.\n", net->n - 1);
        return;
    }

    int *visited = calloc(net->n, sizeof(int));
    int *order = malloc(net->n * sizeof(int));
    bfs(net, a, visited, order);

    if (visited[b]) printf("  Computer %d CAN reach computer %d.\n", a, b);
    else            printf("  Computer %d CANNOT reach computer %d.\n", a, b);

    free(visited);
    free(order);
}

void show_network(Network *net) {
    printf("  Computers: %d, Cables: %d\n", net->n, net->cables);
    for (int i = 0; i < net->n; i++) {
        printf("  %d:", i);
        for (Node *p = net->adj[i]; p; p = p->next) printf(" %d", p->id);
        printf("\n");
    }
}

void print_menu(void) {
    printf("\n===== Network Connectivity Checker =====\n");
    printf(" 1. Add a cable (connection)\n");
    printf(" 2. Check connectivity using BFS\n");
    printf(" 3. Check connectivity using DFS\n");
    printf(" 4. Show unreachable computers\n");
    printf(" 5. Count separate groups (components)\n");
    printf(" 6. Check path between two computers\n");
    printf(" 7. Show network\n");
    printf(" 8. Exit\n");
}

/* ---------------- Main ---------------- */

int main(void) {
    int n;
    do {
        if (!read_int("Enter number of computers (at least 1): ", &n)) return 0;
        if (n < 1) printf("  There must be at least 1 computer.\n");
    } while (n < 1);

    Network *net = create_network(n);
    printf("Network created with computers numbered 0 to %d.\n", n - 1);

    int choice;
    while (1) {
        print_menu();
        if (!read_int("Enter your choice: ", &choice)) break;

        if (choice == 1) {
            int a, b;
            if (!read_int("  Computer A: ", &a)) break;
            if (!read_int("  Computer B: ", &b)) break;
            if (add_connection(net, a, b))
                printf("  Cable added between %d and %d.\n", a, b);
        } else if (choice == 2) {
            check_connectivity(net, 1);
        } else if (choice == 3) {
            check_connectivity(net, 0);
        } else if (choice == 4) {
            show_unreachable(net);
        } else if (choice == 5) {
            count_components(net);
        } else if (choice == 6) {
            check_path(net);
        } else if (choice == 7) {
            show_network(net);
        } else if (choice == 8) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("  Invalid choice. Enter a number from 1 to 8.\n");
        }
    }

    free_network(net);
    return 0;
}
