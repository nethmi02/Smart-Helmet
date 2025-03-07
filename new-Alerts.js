class Alert {
  // Default threshold values for heart rate anomalies
  static hrLowThreshold = 40; // Lower heart rate threshold
  static hrHighThreshold = 150; // Upper heart rate threshold

  /**
   * Sends an SOS alert and executes associated protocol actions
   * @param {string} type - Type of the alert ('heartBeatAnomaly', 'collision')
   * @param {string} userId - Unique user identifier
   * @param {string} gpsLocation - Last known GPS location of the user
   * @param {Date} timestamp - Timestamp of the alert (default to now)
   * @param {Object} alertData - Additional data specific to the alert type
   */
  static async triggerSOS(
    type,
    userId,
    gpsLocation,
    timestamp = new Date(),
    alertData = {}
  ) {
    const alertMessage = this.buildAlertMessage(
      "SOS",
      type,
      userId,
      gpsLocation,
      timestamp,
      alertData
    );
    if (!alertMessage) return;

    this.sendAlert(alertMessage);
    const protocolActions = await this.fetchProtocolActions(userId, "SOS");
    this.executeProtocolActions(alertMessage, userId, protocolActions);
  }

  /**
   * Sends a Non-Critical alert and executes associated protocol actions
   * @param {string} type - Type of the alert ('heartBeatAnomaly', 'moduleFailure')
   * @param {string} userId - Unique user identifier
   * @param {string} gpsLocation - Last known GPS location of the user
   * @param {Date} timestamp - Timestamp of the alert (default to now)
   * @param {Object} alertData - Additional data specific to the alert type
   */
  static async triggerNonCritical(
    type,
    userId,
    gpsLocation,
    timestamp = new Date(),
    alertData = {}
  ) {
    const alertMessage = this.buildAlertMessage(
      "Non-Critical",
      type,
      userId,
      gpsLocation,
      timestamp,
      alertData
    );
    if (!alertMessage) return;

    this.sendAlert(alertMessage);
    const protocolActions = await this.fetchProtocolActions(
      userId,
      "Non-Critical"
    );
    this.executeProtocolActions(alertMessage, userId, protocolActions);
  }

  /**
   * Builds an alert message based on the provided parameters
   * @param {string} alertType - Type of alert ('SOS' or 'Non-Critical')
   * @param {string} type - Specific alert type ('heartBeatAnomaly', 'collision', 'moduleFailure')
   * @param {string} userId - Unique user identifier
   * @param {string} gpsLocation - Last known GPS location
   * @param {Date} timestamp - Alert timestamp
   * @param {Object} alertData - Additional alert-specific data
   * @returns {Object|null} - Constructed alert message or null if invalid type
   */
  static buildAlertMessage(
    alertType,
    type,
    userId,
    gpsLocation,
    timestamp,
    alertData
  ) {
    let alertMessage = {
      type: alertType,
      userId: userId,
      gpsLocation: gpsLocation,
      timestamp: timestamp.toISOString(),
    };

    switch (type) {
      case "heartBeatAnomaly":
        return {
          ...alertMessage,
          alertType: "Heart Beat Anomaly",
          message: alertData.message || "Heart rate anomaly detected.",
          lastBPM: alertData.lastBPM,
          avgBPM: alertData.avgBPM,
        };

      case "collision":
        return {
          ...alertMessage,
          alertType: "Collision",
          message: "Severe collision detected!",
          collisionSeverity: alertData.collisionSeverity,
        };

      case "moduleFailure":
        return {
          ...alertMessage,
          alertType: "Module Failure",
          message: `Module issue detected in ${alertData.moduleName}.`,
          moduleName: alertData.moduleName,
        };

      default:
        console.error("Invalid alert type provided.");
        return null;
    }
  }

  /**
   * Fetches protocol actions from the server
   * @param {string} userId - Unique user identifier
   * @param {string} protocolType - Type of the protocol ('SOS' or 'Non-Critical')
   * @returns {Array} - List of protocol actions
   */
  static async fetchProtocolActions(userId, protocolType) {
    try {
      console.log(
        `Fetching ${protocolType} protocol actions for User ${userId}...`
      );
      const response = await fetch(
        `https://your-api-endpoint/protocolActions?userId=${userId}&protocolType=${protocolType}`
      );

      if (!response.ok) {
        throw new Error(
          `Failed to fetch protocol actions: ${response.statusText}`
        );
      }

      const actions = await response.json();
      console.log("Protocol actions received:", actions);
      return actions;
    } catch (error) {
      console.error("Error fetching protocol actions:", error);
      return [];
    }
  }

  /**
   * Executes protocol actions based on the fetched protocol information
   * @param {Object} alertMessage - The alert message object
   * @param {string} userId - Unique user identifier
   * @param {Array} protocolActions - List of actions to execute
   */
  static executeProtocolActions(alertMessage, userId, protocolActions) {
    protocolActions.forEach((action) => {
      console.log(`Executing action: ${action.actionType}`, action);

      switch (action.actionType) {
        case "sendNotification":
          console.log(
            `Sending notification to ${action.target}: ${action.message}`
          );
          break;

        case "logEvent":
          console.log(`Logging event: ${action.eventName} - ${action.details}`);
          break;

        case "triggerDeviceAction":
          console.log(
            `Triggering device action: ${action.deviceName} - ${action.command}`
          );
          break;

        default:
          console.error(`Unknown action type: ${action.actionType}`);
      }
    });
  }
}
