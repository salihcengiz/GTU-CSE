Mandatory test scenario screenshots for report.tex

Folder name: screenshots (plural), next to report.tex — same as \ScreenshotDir in report.tex.

Place images in this folder. report.tex tries, in order for each step:
  StepN.png, StepN.pdf, stepN.png, stepN.pdf, step-0N.png, step-0N.pdf
(Step 1 uses step-01, Step 2 uses step-02, ... Step 6 uses step-06.)

Step 6 in this repo also accepts Step6-sdterr.png as the first basename (see \MandatoryFigThree in report.tex).

Rename your files to match, or edit \MandatoryFigThree{...}{...}{...} in report.tex.

Step 1: Step1 / step1 / step-01
Step 2: Step2 / step2 / step-02
Step 3: Step3 / step3 / step-03
Step 4: Step4 / step4 / step-04
Step 5: Step5 / step5 / step-05
Step 6: Step6-sdterr / Step6 / step6 / step-06

Run pdflatex from the HW2 directory (same folder as report.tex):
  pdflatex report.tex
  pdflatex report.tex
