/*********************************************************************
 * \page   MinimalClient.cpp
 * \file   MinimalClient.cpp
 * \brief  The *minimal* amount of code required to connect to Motive and get data.
 * For a more complete example with additional functionality, consult the
 * SampleClient.cpp example in the NatNet SDK
 *********************************************************************/

 /*
Copyright 2012 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
*/

// using STL for cross platform sleep
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <unordered_map>
#include <atomic>
#include <csignal>
#include <chrono>

// NatNet SDK includes
#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

#define VERBOSE
#undef VERBOSE

/**
 * @brief A wrapper data structure of sFrameOfMocapData with a timestamp.
 */
struct TimeStampedFrameData {
    std::chrono::steady_clock::time_point arrivalTime;
    const sFrameOfMocapData* frameData;
};

void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData);    // receives data from the server
void PrintData(sFrameOfMocapData* data, NatNetClient* pClient);
void LogData(const TimeStampedFrameData* data);
void PrintDataDescriptions(sDataDescriptions* pDataDefs);

NatNetClient* g_pClient = nullptr;
sNatNetClientConnectParams g_connectParams;
sServerDescription g_serverDescription;
sDataDescriptions* g_pDataDefs = nullptr;
std::atomic<bool> g_running = true;

void signal_handler(int signal) {
    g_running = false;
    std::cout << "Received signal [" << signal << "]. Shutting down..." << std::endl;
}


/**
 * \brief Minimal client example.
 * 
 * \param argc
 * \param argv
 * \return Returns NatNetTypes Error code.
 */
int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    ErrorCode ret = ErrorCode_OK;

    // Parse command-line arguments
    bool is_remote = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--remote") {
            is_remote = true;
        }
    }

    // Create a NatNet client
    g_pClient = new NatNetClient();

    // Set the Client's frame callback handler
    ret = g_pClient->SetFrameReceivedCallback(DataHandler, g_pClient);	

    // Specify client PC's IP address, Motive PC's IP address, and network connection type
    if (!is_remote) {
        g_connectParams.localAddress = "127.0.0.1";
        g_connectParams.serverAddress = "127.0.0.1";
        std::cout << "Connecting to Motive on the same PC." << std::endl;
    } else {
        g_connectParams.serverAddress = "192.168.0.26";
        std::cout << "Connecting to Motive on a remote PC." << std::endl;
    }
    g_connectParams.connectionType = ConnectionType_Multicast;

    // Connect to Motive
    try {
        ret = g_pClient->Connect(g_connectParams);
    } catch (...) {
        std::cerr << "Error connecting to Motive: " << std::endl;
        return 1;
    }
    if (ret != ErrorCode_OK)
    {
        // Connection failed
        printf("Unable to connect to server.  Error code: %d. Exiting.\n", ret);
        return 1;
    }
     
    // Get Motive server description
    memset(&g_serverDescription, 0, sizeof(g_serverDescription));
    ret = g_pClient->GetServerDescription(&g_serverDescription);
    if (ret != ErrorCode_OK || !g_serverDescription.HostPresent)
    {
        printf("Unable to get server description. Error Code:%d.  Exiting.\n", ret);
        return 1;
    }
    else
    {
        printf("Connected : %s (ver. %d.%d.%d.%d)\n", g_serverDescription.szHostApp, g_serverDescription.HostAppVersion[0],
            g_serverDescription.HostAppVersion[1], g_serverDescription.HostAppVersion[2], g_serverDescription.HostAppVersion[3]);
    }

    // Get current active asset list from Motive
    ret = g_pClient->GetDataDescriptionList(&g_pDataDefs);
    if (ret != ErrorCode_OK || g_pDataDefs == NULL)
    {
        printf("Error getting asset list.  Error Code:%d  Exiting.\n", ret);
        return 1;
    }
    else
    {
        PrintDataDescriptions(g_pDataDefs);
    }

    printf("\nClient is connected and listening for data...\n");
    printf("Press Ctrl+C to exit.\n");
    
    // do something on the main app's thread...
    while (g_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up
    if (g_pClient)
    {
        g_pClient->Disconnect();
        delete g_pClient;
        g_pClient = nullptr;
    }
    
    if (g_pDataDefs)
    {
        NatNet_FreeDescriptions(g_pDataDefs);
        g_pDataDefs = NULL;
    }

    return ErrorCode_OK;
}

/**
 * DataHandler called by NatNet on a separate network processing
 * thread whenever a frame of mocap data is available.
 * So at 100 mocap fps, this function should be called ~ every 10ms.
 * \brief DataHandler called by NatNet
 * \param data Input Frame of Mocap data
 * \param pUserData
 * \return 
 */
void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData)
{
    if (!g_running) {
        return;
    }

    try {
        auto arrivalTime = std::chrono::steady_clock::now();
        NatNetClient* pClient = (NatNetClient*)pUserData;

#ifdef VERBOSE
        PrintData(data, pClient);
#endif
        TimeStampedFrameData timestampedData{arrivalTime, data};
        LogData(&timestampedData);
    }
    catch (const std::exception& e) {
        std::cerr << "Error in DataHandler: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error in DataHandler" << std::endl;
    }

    return;
}

/**
 * \brief Print out the current Motive active assets descriptions.
 * 
 * \param pDataDefs
 */
void PrintDataDescriptions(sDataDescriptions* pDataDefs)
{
    printf("Retrieved %d Data Descriptions:\n", pDataDefs->nDataDescriptions);
    for (int i = 0; i < pDataDefs->nDataDescriptions; i++)
    {
        printf("---------------------------------\n");
        printf("Data Description # %d (type=%d)\n", i, pDataDefs->arrDataDescriptions[i].type);
        if (pDataDefs->arrDataDescriptions[i].type == Descriptor_MarkerSet)
        {
            // MarkerSet
            sMarkerSetDescription* pMS = pDataDefs->arrDataDescriptions[i].Data.MarkerSetDescription;
            printf("MarkerSet Name : %s\n", pMS->szName);
            for (int i = 0; i < pMS->nMarkers; i++)
                printf("%s\n", pMS->szMarkerNames[i]);

        }
        else if (pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody)
        {
            // RigidBody
            sRigidBodyDescription* pRB = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
            printf("RigidBody Name : %s\n", pRB->szName);
            printf("RigidBody ID : %d\n", pRB->ID);
            printf("RigidBody Parent ID : %d\n", pRB->parentID);
            printf("Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx, pRB->offsety, pRB->offsetz);

            if (pRB->MarkerPositions != NULL && pRB->MarkerRequiredLabels != NULL)
            {
                for (int markerIdx = 0; markerIdx < pRB->nMarkers; ++markerIdx)
                {
                    const MarkerData& markerPosition = pRB->MarkerPositions[markerIdx];
                    const int markerRequiredLabel = pRB->MarkerRequiredLabels[markerIdx];

                    printf("\tMarker #%d:\n", markerIdx);
                    printf("\t\tPosition: %.2f, %.2f, %.2f\n", markerPosition[0], markerPosition[1], markerPosition[2]);

                    if (markerRequiredLabel != 0)
                    {
                        printf("\t\tRequired active label: %d\n", markerRequiredLabel);
                    }
                }
            }
        }
        else if (pDataDefs->arrDataDescriptions[i].type == Descriptor_Camera)
        {
            // Camera
            sCameraDescription* pCamera = pDataDefs->arrDataDescriptions[i].Data.CameraDescription;
            printf("Camera Name : %s\n", pCamera->strName);
            printf("Camera Position (%3.2f, %3.2f, %3.2f)\n", pCamera->x, pCamera->y, pCamera->z);
            printf("Camera Orientation (%3.2f, %3.2f, %3.2f, %3.2f)\n", pCamera->qx, pCamera->qy, pCamera->qz, pCamera->qw);
        }
        else
        {
            // Unknown
            printf("Unknown data type.\n");
        }
    }
}

/**
 * \brief Print out a single frame of mocap data.
 * 
 * \param data
 * \param pClient
 */
void PrintData(sFrameOfMocapData* data, NatNetClient* pClient)
{
    printf("\n=====================  New Packet Arrived  =============================\n");
    printf("FrameID : %d\n", data->iFrame);
    printf("Timestamp : %3.2lf\n", data->fTimestamp);
    
    // Rigid Bodies
    printf("------------------------\n");
    printf("Rigid Bodies [ Count = %d ]\n", data->nRigidBodies);
    for (int i = 0; i < data->nRigidBodies; i++)
    {
        // params
        bool bTrackingValid = data->RigidBodies[i].params & 0x01;
        int streamingID = data->RigidBodies[i].ID;
        printf("[ID=%d  Error=%3.4f  Tracked=%d]\n", streamingID, data->RigidBodies[i].MeanError, bTrackingValid);
        printf("\tx\ty\tz\tqx\tqy\tqz\tqw\n");
        printf("\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
            data->RigidBodies[i].x,
            data->RigidBodies[i].y,
            data->RigidBodies[i].z,
            data->RigidBodies[i].qx,
            data->RigidBodies[i].qy,
            data->RigidBodies[i].qz,
            data->RigidBodies[i].qw);
    }

    // Labeled markers - this includes all markers (Active, Passive, and 'unlabeled' (markers with no asset but a PointCloud ID)
    bool bUnlabeled;    // marker is 'unlabeled', but has a point cloud ID that matches Motive PointCloud ID (In Motive 3D View)
    bool bActiveMarker; // marker is an actively labeled LED marker
    printf("------------------------\n");
    printf("Markers [ Count = %d ]\n", data->nLabeledMarkers);
    for (int i = 0; i < data->nLabeledMarkers; i++)
    {
        bUnlabeled = ((data->LabeledMarkers[i].params & 0x10) != 0);
        bActiveMarker = ((data->LabeledMarkers[i].params & 0x20) != 0);
        sMarker marker = data->LabeledMarkers[i];
        int modelID, markerID;
        NatNet_DecodeID(marker.ID, &modelID, &markerID);
        char szMarkerType[512];
        if (bActiveMarker)
            strcpy_s(szMarkerType, "Active");
        else if (bUnlabeled)
            strcpy_s(szMarkerType, "Unlabeled");
        else
            strcpy_s(szMarkerType, "Labeled");
        printf("%s Marker [ModelID=%d, MarkerID=%d] [size=%3.2f] [pos=%3.2f,%3.2f,%3.2f]\n",
            szMarkerType, modelID, markerID, marker.size, marker.x, marker.y, marker.z);
    }
}

/**
 * \brief Log the data to a file.
 * 
 * \param data
 */
void LogData(const TimeStampedFrameData* timestampedData) 
{
    static std::unordered_map<std::string, std::ofstream> file_streams;

    try{
        const auto &data = timestampedData->frameData;
        const auto &arrivalTime = timestampedData->arrivalTime;

        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(arrivalTime.time_since_epoch()).count();

        for (int i = 0; i < data->nRigidBodies; i++) {
            const auto& rigid_body = data->RigidBodies[i];
            const std::string rigid_body_name = std::string(g_pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription->szName);
            std::string filename = "rigid_body_" + rigid_body_name + ".csv";

            // Open the file stream if it's not already open
            if (!file_streams.count(filename)) {
                file_streams[filename].open(filename, std::ios::app);
                if (!file_streams[filename]) {
                    std::cerr << "Failed to open file: " << filename << std::endl;
                    continue;
                }
                // Write header if it's a new file
                if (file_streams[filename].tellp() == 0) {
                    file_streams[filename] << "ArrivalTimeUs,ID,Timestamp,X,Y,Z,QX,QY,QZ,QW\n";
                }
            }

            // Prepare the data string
            std::ostringstream oss;
            oss << micros << ','
                << rigid_body_name << ','
                << std::fixed << std::setprecision(7) << data->fTimestamp << ','
                << std::setprecision(9) << rigid_body.x << ','
                << rigid_body.y << ','
                << rigid_body.z << ','
                << std::setprecision(10) << rigid_body.qx << ','
                << rigid_body.qy << ','
                << rigid_body.qz << ','
                << rigid_body.qw << '\n';

            // Write to the file
            file_streams[filename] << oss.str();

            // Check for write errors
            if (!file_streams[filename]) {
                std::cerr << "Failed to write to file: " << filename << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in LogData: " << e.what() << std::endl;
    }
}
