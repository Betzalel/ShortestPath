#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>


#define PI 3.14159
#define RADIUS 6371 // earth radius km



//------------------------------------------------------------------------
typedef struct location
{
    char * city_name;
    double lattitude;
    double longitude;
    struct connection * connections;
    struct location * next;
    struct location * previous;

} location_node;

location_node * HEAD, * TAIL;

//------------------------------------------------------------------------
typedef struct connection
{
    double distance;
    location_node * city;
    struct connection * next;
} connect_node;

//------------------------------------------------------------------------
typedef struct dijk_node
{
    location_node * node;
    double distance;
    struct dijk_node * previous;
    struct dijk_node * next;
} dijk;

//------------------------------------------------------------------------
FILE * open_file();
void parse_file(FILE * file_to_parse);
location_node * create_location_node();
location_node * process_part(char * part);
double to_dec_deg(int deg, int min, int sec);
double to_radians(double degrees);
double to_degrees(double radians);
double calculate_dist_bear(double lat1, double lat2, double lon1, double lon2);
location_node * find_city(char * city);
void add_connection(location_node * node1, location_node * node2, double distance);
void insert_connection(connect_node * pointer, location_node * destination, double distance);
void find_shortest_path(FILE * file);

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void dijksra(char * start, char * end);
void give_distance(dijk * dijk_head, dijk * pointer);
dijk * find_shortest(dijk * pointer);
int elements_in(dijk * pointer);
dijk * make_dijk_graph();
dijk * create_dijk(dijk * pointer, location_node * data);
void insert_dijk(dijk * pointer, dijk * node);
dijk * find_dijk_location(dijk * pointer, location_node * element);
dijk * find_dijk_city(dijk * pointer, char * city);
void dijk_delete(dijk * pointer, dijk * element);
void print_shortest(dijk * element);
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void print_cities();
void print_connections(connect_node * pointer);

//------------------------------------------------------------------------
int main(int argc, char * argv[])
{
    printf("File to populate graph.\n");

    FILE * file = open_file();

    printf("\n*********************************\n");
    printf("\nCities Distance and Bearing\n\n");

    parse_file(file);

    printf("\n*********************************\n");
    printf("\nCities and Connections\n\n");
    print_cities();

    printf("\n*********************************\n");
    printf("\nFile for shortest path.\n");

    FILE * file2 = open_file();

    printf("\n*********************************\n");
    find_shortest_path(file2);

    printf("\n* * * Program complete * * *\n");

    return 0;

}

//------------------------------------------------------------------------
// Requets a file name and opens the file.
// Returns the opened file.
FILE * open_file()
{
    char file_name[100];
    printf("Enter the file to open (limit 100 characters): ");
    scanf("%s", file_name);

    FILE * file_open = fopen(file_name, "r");

    if (file_open == NULL)
    {
        printf("* * * Error opening file. * * *\n");
        exit(-1);
    }

    printf("File opened successfully.\n");

    return file_open;

}

//------------------------------------------------------------------------
// Takes a file line by line and splits it by the '|' delimiter.
void parse_file(FILE * file_to_parse)
{
    HEAD = create_location_node();
    HEAD->city_name = NULL;
    HEAD->next = NULL;
    HEAD->previous = NULL;

    TAIL = HEAD;

    if (file_to_parse)
    {


        char * line;
        char buffer[100];


        while ((line = fgets(buffer, 100, file_to_parse)))
        {


            char part1[50] = {'\0'};
            char part2[50] = {'\0'};

            while(*line != '|')
            {
                char to_add[2] = {(char)*line, '\0'};
                strcat(part1,to_add);
                line++;
            }
            line++;
            while(*line != '\0' && *line != '\n' )
            {
                char to_add[2] = {(char)*line, '\0'};
                strcat(part2,to_add);
                line++;
            }

            location_node * node1 = process_part(part1);
            location_node * node2 = process_part(part2);

            printf("%s to %s\n", node1->city_name, node2->city_name);

            double distance = calculate_dist_bear(node1->lattitude, node2->lattitude, node1->longitude, node2->longitude);


            add_connection(node1, node2, distance);

        }
    }


}

//------------------------------------------------------------------------
// Creates a location_node and checks to ensure it is correct.
// Returns the created location_node.
location_node * create_location_node()
{
    location_node * node = malloc(sizeof(location_node));

    if (node == NULL)
    {
        printf("\tNode creation failled at malloc for location_node.\n");
        exit(-1);
    }

    node->connections = malloc(sizeof(connect_node));

    if (node->connections == NULL)
    {
        printf("\tNode creation failed at malloc for connect_node.\n");
        exit(-1);
    }


    return node;
}

//------------------------------------------------------------------------
// Proccesses a string to find city_name, lat and lon.
// Returns a location_node with the information it it.
location_node * process_part(char * part)
{
    char * city = malloc(sizeof(char) * 25);
    if (city == NULL)
    {
        printf("malloc failed for city.\n");
        exit(-1);
    }

    city[0] = '\0';

    int lat_deg, lat_min, lat_sec;
    int lon_deg, lon_min, lon_sec;
    char ns, ew;

    location_node * temp;

    while(*part != '=')
    {
        char to_add[2] = {(char)*part,'\0'};
        strcat(city, to_add);
        part++;
    }

    if((temp = find_city(city)) != NULL) return temp;


    part++; part++; part++;

    sscanf(part, "%d:%d:%d %c %d:%d:%d %c", &lat_deg, &lat_min, &lat_sec, &ns, &lon_deg, &lon_min, &lon_sec, &ew);

    double lat = to_dec_deg(lat_deg,lat_min,lat_sec);
    double lon = to_dec_deg(lon_deg,lon_min,lon_sec);

    if (ns == 'S') lat = lat * -1;
    if (ew == 'w') lon = lon * -1;

    temp = create_location_node();

    temp->city_name = city;
    temp->lattitude = lat;
    temp->longitude = lon;
    temp->next = NULL;
    if (HEAD->city_name == NULL)
    {
        temp->previous = NULL;
        HEAD = temp;
    }
    else
    {
        temp->previous = TAIL;
        TAIL->next = temp;
    }
    TAIL = temp;

    return temp;

}

//------------------------------------------------------------------------
// Finds a city in the location list.
// Returns a location_node if found or NULL if not found.
location_node * find_city(char * city)
{
    if (HEAD->city_name == NULL) return NULL; // for first case.

    location_node * temp = HEAD;
    while ((strcmp(temp->city_name, city) != 0) && (temp->next != NULL)) temp = temp->next;
    if((strcmp(temp->city_name, city) == 0))
        return temp;

    return NULL;
}

//------------------------------------------------------------------------
// Uses insert_connection to add connections to both provided nodes.
// Return nothing.
void add_connection(location_node * node1, location_node * node2, double distance)
{
    insert_connection(node1->connections, node2, distance);
    insert_connection(node2->connections, node1, distance);
}

//------------------------------------------------------------------------
// Creates a connection node to the list of nodes.
// Returns nothing.
void insert_connection(connect_node * pointer, location_node * destination, double distance)
{
    while(pointer->next != NULL) pointer = pointer->next;

    pointer->next = malloc(sizeof(connect_node));

    if(pointer->next == NULL)
    {
        printf("\tNode creation failed for connect_node.\n");
        exit(-1);
    }

    pointer = pointer->next;
    pointer->distance = distance;
    pointer->city = destination;
    pointer->next = NULL;
}


void find_shortest_path(FILE * file)
{
    if (file)
    {



        char * line;
        char buffer[100];


        while ((line = fgets(buffer, 100, file)))
        {


            char city_start[50] = {'\0'};
            char city_end[50] = {'\0'};

            while(*line != '-')
            {
                char to_add[2] = {(char)*line, '\0'};
                strcat(city_start,to_add);
                line++;
            }
            line++;
            while(*line != '\0' && *line != '\n' )
            {
                char to_add[2] = {(char)*line, '\0'};
                strcat(city_end,to_add);
                line++;
            }

            dijksra(city_start, city_end);
        }
    }
}




//************************************************************************
// The following functions deal with the math of finding the bearing and
// distance between two cities.
//************************************************************************

//------------------------------------------------------------------------
// Calculates both distance and bearing.
// Returns distance in km.
double calculate_dist_bear(double lat1, double lat2, double lon1, double lon2)
{
    double y, x, brng, delta_lat, delta_lon; // delta = change in

    lat1 = to_radians(lat1);
    lat2 = to_radians(lat2);
    lon1 = to_radians(lon1);
    lon2 = to_radians(lon2);

    delta_lat = lat2-lat1;
    delta_lon = lon2-lon1;

    double a, c, d;

    a = sin(delta_lat / 2) * sin(delta_lat / 2) + sin(delta_lon / 2) * sin(delta_lon / 2) * cos(lat1) * cos(lat2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    d = RADIUS * c;

    printf("\tDistance: %.3fl km\t", d);

    y = sin(delta_lon) * cos(lat2);
    x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(delta_lon);
    brng = atan2(y, x);

    brng = to_degrees(brng);

    if (brng < 0) brng = 360 + brng;

    printf("Bearing: %.3fl degrees from North.\n", brng);

    return d;
}

//------------------------------------------------------------------------
// Converts degrees to radians.
double to_radians(double degrees)
{
    return (degrees * (PI / 180));
}

//------------------------------------------------------------------------
// Converts radians to degrees
double to_degrees(double radians)
{
    return (radians * (180 / PI));
}

//------------------------------------------------------------------------
// Converts DMS to Dec Deg.
double to_dec_deg(int deg, int min, int sec)
{
    return (deg + ((double)((min * 60) + sec) / 3600));
}


//************************************************************************
// The following functions deal with printing out different nodes.
// This is mainly for testing purposes.
//************************************************************************

//------------------------------------------------------------------------
void print_cities()
{
    if(HEAD->city_name == NULL)
    {
        printf("* * * No cities available * * *\n");
        return;
    }

    location_node * temp = HEAD;

    while(temp->next != NULL)
    {
        printf("City: %s\tLat: %.3fl\tLon: %.3fl\n", temp->city_name, temp->lattitude, temp->longitude);
        print_connections(temp->connections);
        temp = temp->next;
    }

    // Fencepost.
    printf("City: %s\tLat: %.3fl\tLon: %.3fl\n", temp->city_name, temp->lattitude, temp->longitude);
    print_connections(temp->connections);
}

void print_connections(connect_node * pointer)
{
    int count = 1;

    pointer = pointer->next; // Takes care of dummy node.


    while(pointer->next != NULL)
    {
        printf("\tCity %d: %s\tDistance: %.3fl km\n", count, (pointer->city)->city_name, pointer->distance);
        pointer = pointer->next;
        count++;
    }

    printf("\tCity %d: %s\tDistance: %.3fl km\n", count, (pointer->city)->city_name, pointer->distance);

}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void dijksra(char * start, char * end)
{

    printf("\nShortest path from %s to %s.\n", start, end);

    location_node * node_start = find_city(start);
    location_node * node_end = find_city(end);

    if (node_start == NULL)
    {
        printf("* * * The start city is not in the list.\n\n");
        return;
    }
    if (node_end == NULL)
    {
        printf("* * * The end city is not in the list.\n\n");
        return;
    }

    dijk * dijk_head = make_dijk_graph();

    dijk * dijk_start = find_dijk_city(dijk_head, start);
    dijk * dijk_end = find_dijk_city(dijk_head, end);

    dijk * found = malloc(sizeof(dijk));

    if (found == NULL)
    {
        printf("malloc failed in dijksra()\n");
        exit(-1);
    }

    found->next = NULL;

    dijk_start->distance = 0.0;

    while(elements_in(dijk_head))
    {
        dijk * element = find_shortest(dijk_head);
        dijk_delete(dijk_head, element);
        insert_dijk(found, element);

        if(element == dijk_end)
        {
            print_shortest(element);
            return;
        }
        give_distance(dijk_head, element);
    }

}

void print_shortest(dijk * element)
{
    dijk * temp = element;

    printf("\tFrom Destination:\n");

    while(temp->previous != NULL)
    {
        printf("\t\t%s\n", (temp->node)->city_name);
        temp = temp->previous;
    }

    printf("\t\t%s\n\n", (temp->node)->city_name);

    printf("\tTotal Distance: %fl\n", element->distance);

}

void give_distance(dijk * dijk_head, dijk * pointer)
{
    connect_node * connect = (pointer->node)->connections;

    double node_distance = pointer->distance;

    while(connect->next != NULL)
    {
        connect = connect->next;
        dijk * temp = find_dijk_location(dijk_head, connect->city);
        if(temp != NULL)
        {

            if(temp->distance > (node_distance + connect->distance))
            {
                temp->distance = (node_distance + connect->distance);
                temp->previous = pointer;
            }
        }
    }

}

dijk * find_shortest(dijk * pointer)
{
    dijk * temp = pointer->next;

    while(pointer->next != NULL)
    {
        pointer = pointer->next;
        if(pointer->distance < temp->distance) temp = pointer;
    }

    return temp;
}


int elements_in(dijk * pointer)
{
    if(pointer->next != NULL)
     {
        return 1;
     }
    else
        return 0;
}

dijk * make_dijk_graph()
{
    dijk * head = malloc(sizeof(dijk));

    if (head == NULL)
    {
        printf("malloc failed in make_dijk_graph()\n");
        exit(-1);
    }

    dijk * tail = head;

    location_node * temp = HEAD;

    while(temp->next != NULL)
    {
        tail = create_dijk(tail, temp);
        temp = temp->next;
    }

    tail = create_dijk(tail, temp);

    return head;
}

// Creates a dijk node from a location_node.
dijk * create_dijk(dijk * pointer, location_node * data)
{
    // Get to the last one. Can just pass tail to make it faster.
    while (pointer->next != NULL) pointer = pointer->next;

    // Allocating memory. Must do this in C.
    pointer->next = malloc(sizeof(dijk)); //Allocated memory will become the ->next for current node.

    // Null check on pointer->next.
    if (pointer->next == NULL)
    {
        printf("\tNode creation failled at malloc.\n");
        exit(-1);
    }

    pointer = pointer->next;
    pointer->node = data;
    pointer->distance = DBL_MAX;
    pointer->previous = NULL;
    pointer->next = NULL;

    return pointer;
}

// Adds a dijk to the end of the list.
void insert_dijk(dijk * pointer, dijk * node)
{
    while(pointer->next != NULL) pointer = pointer->next;

    pointer->next = node;

    if (node->next != NULL) node->next = NULL;
}

dijk * find_dijk_location(dijk * pointer, location_node * element)
{
    while((pointer->next != NULL) && ((pointer->next)->node != element))
        pointer = pointer->next;

    if((pointer->next == NULL) && (pointer->node != element))
    {
        return NULL;
    }
    else
        return pointer->next;
}

// Finds a dijk by the city name.
dijk * find_dijk_city(dijk * pointer, char * city)
{
    while((pointer->next != NULL) && (strcmp(((pointer->next)->node)->city_name, city) != 0))
        pointer = pointer->next;

    if(strcmp(((pointer->next)->node)->city_name, city) == 0)
        return pointer->next;
    else
        return NULL;
}


// Removes a dijk from the list
void dijk_delete(dijk * pointer, dijk * element)
{
     // Find the node. Could traverse from back with some alteration.
    while(pointer->next != NULL)
    {
        if(pointer->next != element)
            pointer = pointer->next;
        else
        {
            pointer->next = (pointer->next)->next;
            return;

        }
    }

}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
