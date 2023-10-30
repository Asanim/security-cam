# SQL
import sqlite3
import pandas as pd
from datetime import datetime
import json

# store app amJSONData in default program directory
#     Note: the directory must be owned by the Linux User Group: gcc_user to access!
#     The location of the root directory is in the local GreenGrass module directory
#     For instance: /greengrass/v2/work/com.example.ipcgreengrass/ 

def convertTimeToAWSDateTime(iTimestamp):
    sPythonTimestamp = datetime.fromtimestamp(iTimestamp).isoformat()
    sTimestampTemp = sPythonTimestamp.split('.',1)
    sAWSTimestamp = sTimestampTemp[0] +'.'+sTimestampTemp[1][0:3]+'Z'
    return sAWSTimestamp


# SQL service class
#   * Opens a connection to the amJSONDatabase 
#   * encapsulates all amJSONDatabase interactions 
#   * retains a enum of tables
#   * lists all schemas

# Note: SQL must be connected to in the same thread 
#       (hence this SQL object must be craeted in the same thread it is used)

# Note: for python maps, the m datatype prefix is used
class SQLService ():
    def __init__(self, sDatabaseName, iDatabaseSize):
        # make static so we do not have to connect to SQL every time
        # Create SQL server
        self.oDatabaseConnection = sqlite3.connect(sDatabaseName)
        self.oDatabaseCursor = self.oDatabaseConnection.cursor()

        # Store at most iDatabaseSize records
        self.iDatabaseSize = iDatabaseSize

        # Check if the tables are created. If not, create them
        self.createDetectionEventTables()

    def checkTableExists(self, sTableName):
        oResult = self.oDatabaseCursor.execute("SELECT name FROM sqlite_master WHERE name='"+sTableName+"'")
        if oResult.fetchone() is None:
                return False
        return True

    # Checks if the table exists, if not, create a new one. 
    def createDetectionEventTables(self):
        if (not self.checkTableExists("stBoundingBox")):
            # Create stBoundingBox
            self.oDatabaseCursor.execute(
            """
            CREATE TABLE stBoundingBox (
                iBoundingBoxID INTEGER PRIMARY KEY AUTOINCREMENT,
                iDatabaseRowID INTEGER NOT NULL UNIQUE,
                iDetectionID INTEGER,

                iXMin INTEGER,
                iYMin INTEGER,
                iXMax INTEGER,
                iYMax INTEGER
            );
            """)

        if (not self.checkTableExists("stDetection")):
            # Create stDetection
            self.oDatabaseCursor.execute(
            """
            CREATE TABLE stDetection (
                iDetectionID INTEGER PRIMARY KEY AUTOINCREMENT,
                iDatabaseRowID INTEGER NOT NULL UNIQUE,
                iDetectionEventID INTEGER,
                
                iCameraZoneID INTEGER,
                iConfidence INTEGER,
                sType TEXT
            );
            """)

        if (not self.checkTableExists("stPosition")):
            # Create stPosition
            self.oDatabaseCursor.execute(
            """
            CREATE TABLE stPosition (
                iPositionID INTEGER PRIMARY KEY AUTOINCREMENT,
                iDatabaseRowID INTEGER NOT NULL UNIQUE,
                iDetectionEventID INTEGER,

                fLatitude REAL,
                fLongitude REAL
            );
            """)

        if (not self.checkTableExists("stDetectionEvent")):
            # Create stDetectionEvent
            self.oDatabaseCursor.execute(
            """
            CREATE TABLE stDetectionEvent (
                iDetectionEventID INTEGER PRIMARY KEY AUTOINCREMENT,
                iDatabaseRowID INTEGER NOT NULL UNIQUE,

                idCompanyPartition TEXT,
                iTimestamp INTEGER,
                idMachine TEXT,
                iDuration INTEGER,
                tUpdatedAt TEXT,
                tCreatedAt TEXT
            );
            """)
            self.oDatabaseConnection.commit() 


    def insertDetectionEvent(self, amDetectionEventData):
        self.oDatabaseCursor.execute(
        "REPLACE INTO stDetectionEvent (iDatabaseRowID, idCompanyPartition, iTimestamp, idMachine, iDuration, tUpdatedAt, tCreatedAt)"
            +"VALUES ((SELECT COALESCE(MAX(iDetectionEventID), 0) % "+str(self.iDatabaseSize)+" + 1 FROM stDetectionEvent AS t), "
            +"'"+str(amDetectionEventData['idCompanyPartition'])+"', "
            +"'"+str(amDetectionEventData['iTimestamp'])+"', "
            +"'"+str(amDetectionEventData['idMachine'])+"', "
            +"'"+str(amDetectionEventData['iDuration'])+"', "
            +"'"+str(amDetectionEventData['tUpdatedAt'])+"', "
            +"'"+str(amDetectionEventData['tCreatedAt'])+"')"
        )
        
        for amDetectionItem in amDetectionEventData['stDetections']:
            self.oDatabaseCursor.execute(
            "REPLACE INTO stDetection (iDatabaseRowID, iDetectionEventID, iCameraZoneID, iConfidence, sType)"
                +"VALUES ((SELECT COALESCE(MAX(iDetectionID), 0) % "+str(self.iDatabaseSize)+" + 1 FROM stDetection AS t), (SELECT COALESCE(MAX(iDetectionEventID), 0)FROM stDetectionEvent AS t), "
                +"'"+str(amDetectionItem['iCameraZoneID'])+"', "
                +"'"+str(amDetectionItem['iConfidence'])+"', "
                +"'"+str(amDetectionItem['sType'])+"')"
            )

            for amBoundingBox in amDetectionItem['stBoundingBox']:
                self.oDatabaseCursor.execute(
                "REPLACE INTO stBoundingBox (iDatabaseRowID, iDetectionID, iXMin, iYMin, iXMax, iYMax) "
                    +"VALUES ((SELECT COALESCE(MAX(iBoundingBoxID), 0) % "+str(self.iDatabaseSize)+" + 1 FROM stBoundingBox AS t), (SELECT COALESCE(MAX(iDetectionID), 0) FROM stDetection AS t), "
                    +"'"+str(amBoundingBox['iXMin'])+"', "
                    +"'"+str(amBoundingBox['iYMin'])+"', "
                    +"'"+str(amBoundingBox['iXMax'])+"', "
                    +"'"+str(amBoundingBox['iYMax'])+"')"
                )
        for amPosition in amDetectionEventData['stPosition']:
            self.oDatabaseCursor.execute(
            "REPLACE INTO stPosition (iDatabaseRowID, iDetectionEventID, fLatitude, fLongitude) "
                +"VALUES ((SELECT COALESCE(MAX(iPositionID), 0) % "+str(self.iDatabaseSize)+" + 1 FROM stPosition AS t), (SELECT COALESCE(MAX(iDetectionEventID), 0) FROM stDetectionEvent AS t), "
                +"'"+str(amPosition['fLatitude'])+"', "
                +"'"+str(amPosition['fLongitude'])+"')"
            )
        self.oDatabaseConnection.commit() 

    def readDetectionEvent (self):
        oQuery = self.oDatabaseCursor.execute("""
            SELECT 
                stDetectionEvent.iDetectionEventID,
                stDetectionEvent.iDatabaseRowID,
                idCompanyPartition,
                iTimestamp,
                idMachine,
                iDuration,
                tUpdatedAt,
                tCreatedAt,
                
                stDetection.iCameraZoneID,
                stDetection.iConfidence,
                stDetection.sType,

                stPosition.fLatitude,
                stBoundingBox.iXMin,
                stPosition.fLongitude,

                stBoundingBox.iYMin,
                stBoundingBox.iXMax,
                stBoundingBox.iYMax
            FROM 
                stDetectionEvent
                INNER JOIN stDetection ON stDetection.iDetectionEventID = stDetectionEvent.iDetectionEventID
                INNER JOIN stPosition ON stPosition.iDetectionEventID = stDetectionEvent.iDetectionEventID
                INNER JOIN stBoundingBox ON stBoundingBox.iDetectionID = stDetection.iDetectionID;
        
        """
        )

        data = oQuery.fetchall()
        print(data)