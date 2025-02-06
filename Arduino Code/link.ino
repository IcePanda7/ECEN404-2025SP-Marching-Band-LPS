#include "link.h"                                                                 // Adding the link header file

struct MyLink *init_link(){                                                       //              
  struct MyLink *p = (struct MyLink *)malloc(sizeof(struct MyLink));              //
  p->next = NULL;                                                                 // No device after
  p->anchor_addr = 0;                                                             // Initialize device address
  p->range[0] = 0.0;                                                              // Initialize location
  p->range[1] = 0.0;                                                              // Initialize location
  p->range[2] = 0.0;                                                              // Initialize location
  return p;                                                                       // Return device
}



void add_link(struct MyLink *p, uint16_t addr){                                   // Function to add a device
  struct MyLink *temp = p;                                                        // Create a temperary
  while (temp->next != NULL){                                                     // Iterate through the devices
    temp = temp->next;                                                            // Go to the next device
  }

  // Create an anchor                                                 ********************************
  struct MyLink *a = (struct MyLink *)malloc(sizeof(struct MyLink));              // Creating new device
  a->anchor_addr = 0.0;                                                           // Initializing anchor address
  a->range[0] = 0.0;                                                              // Initializing location
  a->range[1] = 0.0;                                                              // Initializing location
  a->range[2] = 0.0;                                                              // Initializing location
  a->dbm = 0.0;                                                                   // Initialize dbm
  a->next = NULL;                                                                 // Set the next device to NULL

  temp->next = a;                                                                 // Add new device to the end of the list
  return;                                                                         // End function
}



struct MyLink *find_link(struct MyLink *p, uint16_t addr){                        // Data type to find a device
  // Check for valid inputs
  if (addr == 0){ return NULL; }                                                  // Check for valid address
  if(p->next == NULL){ return NULL; }                                             // Check that there are devices

  struct MyLink *temp = p;                                                        // Create a temperary
  while (temp->next != NULL){                                                     // Iterate through devices
    temp = temp->next;                                                            // Go to the next device
    if (temp->anchor_addr == addr){                                               // Check if the device's address matches the desired address
      return temp;                                                                // Return the device
    }
  }
  Serial.println("find-link:Can't find addr");                                    // Print no device with desired address is found
  return NULL;                                                                    // End function
}



void update_link(struct MyLink *p, uint16_t addr, floar range, float dbm){        // Function to update the location
  struct MyLink *temp find_link(p, addr);                                         // A temperary of the device's location that is being updated
  if (temp != NULL){                                                              // Check that there is a device
    temp->range[2] = temp->range[1];                                              // Move the location
    temp->range[1] = temp->range[0];                                              // Move the location
    temp->range[0] = (range + temp->range[1] + temp->range[2]) / 3;               // Recalculate the location
    temp->dbm = dbm;                                                              // Update the dbm
    return;                                                                       // End function
  }
  else{                                                                           // If no device
    return;                                                                       // End function
  }
}



void print_link(struct MyLink *p){                              // Function to print the contents of the device
  struct MyLink *temp = p;                                      // Create a temperary
  while (temp->next != NULL){                                   // Iterate through all the devices
    Serial.println(temp->next->anchor_addr, HEX);               // Print the address
    Serial.println(temp->next->range[0]);                       // Print the average distance
    Serial.println(temp->next->dbm);                            // Print the dbm
    temp = temp->next;                                          // Go to the next device
  }
  return;                                                       // End function
}



void delete_link(struct MyLink *p, uint16_t addr){              // Function to delete a device
  if (addr == 0){ return; }                                     // Check for a proper device address

  struct MyLink *temp = p;                                      // Create a temperary
  while(temp->next != NULL){                                    // Iterate through the devices
    if(temp->next->anchor_addr == addr){                        // Check if the address matches the address that is being deleted
      struct MyLink *del = temp->next;                          // Create a temperary pointer
      temp->next = del->next;                                   // Remove the pointer
      free(del);                                                // Free up memory
      return;                                                   // End function
    }
    temp = temp->next;                                          // If address doesn't match, go to next device
  }
  return;                                                       // End function
}

