from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
import pyperclip as pc
import os


def prepare_test(url, pagename):
    browser = webdriver.Firefox()
    browser.get(url)
    assert pagename in browser.title
    return browser


def run_test(browser, instructions, user_input, expected_output):
    load_instructions_button = browser.find_element(By.ID, "load-instructions-button")
    load_instructions_button.click()

    instructions_textarea = browser.find_element(By.ID, "instructions-textarea")
    instructions_textarea.click()
    instructions_textarea.clear()
    pc.copy("".join(instructions))
    instructions_textarea.send_keys(Keys.CONTROL, "v")

    save_instructions_button = browser.find_element(By.ID, "save-instructions-button")
    save_instructions_button.click()

    input_textarea = browser.find_element(By.ID, "input-textarea")
    input_textarea.click()
    input_textarea.clear()
    pc.copy("".join(user_input))
    input_textarea.send_keys(Keys.CONTROL, "v")

    play_button = browser.find_element(By.ID, "play-button")
    try:
        play_button.click()

        output_textarea = browser.find_element(By.ID, "output-textarea")
        output = output_textarea.get_attribute("value")

        return output == "".join(expected_output)
    except Exception as e:
        return False


def main():
    browser = prepare_test("http://localhost:3000", "PL/0")

    number_of_test_cases = len([name for name in os.listdir(".") if name.startswith("test_case_")])
    with open("results.txt", "w") as fp_res:
        for i in range(1, number_of_test_cases + 1):
            fp_res.write("Test case " + str(i) + ": ")
            with open("test_case_" + str(i) + "/instructions.txt") as fp:
                instructions = fp.readlines()

            with open("test_case_" + str(i) + "/input.txt") as fp:
                user_input = fp.readlines()

            with open("test_case_" + str(i) + "/expected_output.txt") as fp:
                expected_output = fp.readlines()

            res = run_test(browser, instructions, user_input, expected_output)

            if res:
                fp_res.write("Test passed\n")
            else:
                fp_res.write("Test failed\n")

if __name__ == "__main__":
    main()
