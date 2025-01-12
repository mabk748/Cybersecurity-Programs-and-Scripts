#include <iostream>
#include <string>

using namespace std;

// Function to check password strength
string checkPasswordStrength(const string& password) {
    // Criteria flags
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    int minLength = 8;

    // Special characters for validation
    string specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/";

    // Check each character in the password
    for (char ch : password) {
        if (isupper(ch)) hasUpper = true;
        else if (islower(ch)) hasLower = true;
        else if (isdigit(ch)) hasDigit = true;
        else if (specialChars.find(ch) != string::npos) hasSpecial = true;
    }

    // Check overall strength
    int criteriaMet = 0;
    if (hasUpper) criteriaMet++;
    if (hasLower) criteriaMet++;
    if (hasDigit) criteriaMet++;
    if (hasSpecial) criteriaMet++;
    if (password.length() >= minLength) criteriaMet++;

    // Provide feedback based on the score
    if (criteriaMet == 5) {
        return "Strong";
    } else if (criteriaMet >= 3) {
        return "Moderate";
    } else {
        return "Weak";
    }
}

// Function to provide improvement suggestions
void suggestImprovements(const string&password) {
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    int minLength = 8;

    string specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/";

    for (char ch : password) {
        if (isupper(ch)) hasUpper = true;
        else if (islower(ch)) hasLower = true;
        else if (isdigit(ch)) hasDigit = true;
        else if (specialChars.find(ch) != string::npos) hasSpecial = true;
    }

    cout << "Suggestions for improvement:\n";
    if (password.length() < minLength) {
        cout << "- Make the passord at least" << minLength << " characters long.\n";
    }
    if (!hasUpper) {
        cout << "- Add at least one uppercase letter.\n";
    }
    if (!hasLower) {
        cout << "- Add at least one lowercase letter.\n";
    }
    if (!hasDigit) {
        cout << "- Add at least one numeric digit.\n";
    }
    if (!hasSpecial) {
        cout << "- Add at least one special character (e.g., " << specialChars << ").\n";
    }
}

int main() {
    string password;

    cout << "Enter a password to check its strength: ";
    cin >> password;

    // Check the password strength
    string strength = checkPasswordStrength(password);
    cout << "Password strength: " << strength << endl;

    // Provide suggestions if the password id not strong
    if (strength != "Strong") {
        suggestImprovements(password);
    }

    return 0;
}