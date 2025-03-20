document.addEventListener("DOMContentLoaded", function () {
    const conversationDiv = document.getElementById("conversation");
    const messageInput = document.querySelector("input[name='message']");
    const sendButton = document.querySelector("button[id='send']");

    // Function to update the conversation on the page
    function updateConversation() {
        fetch("/get_conversation")
            .then(response => response.text())
            .then(data => {
                conversationDiv.innerHTML = data;
            })
            .catch(error => console.error("Error fetching conversation:", error));
    }

    // Function to send a message to ChatGPT
    function sendMessage() {
        const message = messageInput.value.trim();
        console.log(message);
        if (message) {
            fetch("/send", {
                method: "POST",
                headers: {
                    "Content-Type": "application/x-www-form-urlencoded",
                },
                body: "message=" + encodeURIComponent(message),
            })
                .then(response => {
                    if (response.ok) {
                        messageInput.value = ""; // Clear the input field
                        updateConversation(); // Update the conversation
                    }
                })
                .catch(error => console.error("Error sending message:", error));
        }
    }

    // Event listener for the send button
    sendButton.addEventListener("click", function (event) {
        event.preventDefault(); // Prevent form submission
        sendMessage();
    });

    // Periodically update the conversation (every 2 seconds)
    setInterval(updateConversation, 2000);

    // Function to update the digital display with displaySpeed
    function updateDigitalDisplay() {
        fetch("/get_rpm")
            .then(response => response.text())
            .then(data => {
                document.getElementById("digital-display").textContent = `${parseFloat(data)} RPM`;
            })
            .catch(error => console.error("Error fetching RPM:", error));
    }

    // Update the digital display every second
    setInterval(updateDigitalDisplay, 1000);
});