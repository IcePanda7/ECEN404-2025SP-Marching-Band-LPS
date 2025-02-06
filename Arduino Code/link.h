#include <Arduino.h>                  // Adding the Arduino header file

struct MyLink{                        // Creating a data type
  uint16_t anchor_addr;               // Contains the device address
  float range[3];                     // Contains three location datas
  float dbm;                          // Contains the dbm
  struct MyLink *next;                // Contains the pointer to the next device
};

struct MyLink *init_link();                                                   // Initialization
void add_link(struct MyLink *p, uint16_t addr);                               // Add a device
struct MyLink *find_link(struct MyLink *p, uint_t addr);                      // Find a device
void update_link(struct MyLink *p, uint_t addr, float range, float dbm);      // Update the location of a device
void print_link(struct MyLink *p);                                            // Print the contents in the device
void delete_link(struct MyLink *p, uint16_t addr);                            // Delete a device
