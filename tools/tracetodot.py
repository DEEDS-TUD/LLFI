#! /usr/bin/env python3

# tracetodot.py
# Author: Yilun Song
# This python script is part of the LLFI tracing system
# This script generates all intermediate and final trace files needed to view
# trace propogation flows.
#
# Requires: Please run this scripts in the folder that contains the llfi trace
#           files e.g. the llfi_stat_output folder by default
# Output: Generate trace different report files and its .dot files to the
#         folder trace_report_output


"""
%(PROG)s needs to be called in the folder that contains the llfi trace files
(e.g. /llfi_stat_output)

The trace different report files and .dot files will be generated to the
folder trace_report_output in parallel with the folder llfi_stat_output

Usage: %(PROG)s [OPTIONS]

List of options:

--help(-h):             Show help information
"""


import sys
import os
import argparse
import subprocess
import multiprocessing as mp
import itertools as it


PROG = os.path.basename(sys.argv[0])

QUICK_MODE = False


def parseArgs():
    parser = argparse.ArgumentParser(
        description=__doc__ % globals())
    parser.add_argument(
        '--no-dot', action='store_true',
        help='Do not generate dot files for diff reports.')
    parser.add_argument(
        '--quick-report', action='store_true',
        help='USe quick stop mode for diff report generation. '
             'Implies --no-dot.')

    args = parser.parse_args()
    if args.quick_report:
        global QUICK_MODE
        QUICK_MODE = True
        args.no_dot = True
    return args


def findPath():
    global currentpath, scriptdir

    currentpath = os.getcwd()
    #print (currentpath)

    scriptdir = os.path.dirname(os.path.abspath(__file__))


def makeTraceOutputFolder():
    global traceOutputFolder, goldenTraceFilePath
    traceOutputFolder = os.path.abspath(
        os.path.join(currentpath, "../trace_report_output"))
    #print (traceOutputFolder)
    goldenTraceFilePath = os.path.abspath(os.path.join(
        currentpath, "../baseline/llfi.stat.trace.prof.txt"))
    if not os.path.exists(traceOutputFolder):
        os.makedirs(traceOutputFolder)
    else:
        # Remove the contents in traceOutputFolder
        for f in os.listdir(traceOutputFolder):
            file_path = os.path.join(traceOutputFolder, f)
            if os.path.isfile(file_path):
                os.unlink(file_path)
    if not os.path.isfile(goldenTraceFilePath):
        print("Cannot find golden Trace File 'llfi.stat.trace.prof.txt'")


def executeTraceDiff():
    log_path = os.path.abspath(
        os.path.join(traceOutputFolder, 'generate-diff.log'))
    # Parse the goldenTraceFile path
    # tempgoldenTraceFilePath = goldenTraceFilePath
    # while "(" in tempgoldenTraceFilePath and not r"\(" in tempgoldenTraceFilePath:
    #     tempgoldenTraceFilePath = tempgoldenTraceFilePath[:tempgoldenTraceFilePath.find(
    #         "(")] + r'\(' + tempgoldenTraceFilePath[tempgoldenTraceFilePath.find("(") + 1:]
    # while ")" in tempgoldenTraceFilePath and not r"\)" in tempgoldenTraceFilePath:
    #     tempgoldenTraceFilePath = tempgoldenTraceFilePath[:tempgoldenTraceFilePath.find(
    #         ")")] + r'\)' + tempgoldenTraceFilePath[tempgoldenTraceFilePath.find(")") + 1:]
    # tempScriptdir = scriptdir
    # # Parse the scriptdir path
    # while "(" in tempScriptdir and not r"\(" in tempScriptdir:
    #     tempScriptdir = tempScriptdir[:tempScriptdir.find(
    #         "(")] + r'\(' + tempScriptdir[tempScriptdir.find("(") + 1:]
    # while ")" in tempScriptdir and not r"\)" in tempScriptdir:
    #     tempScriptdir = tempScriptdir[:tempScriptdir.find(
    #         ")")] + r'\)' + tempScriptdir[tempScriptdir.find(")") + 1:]
    # temptraceOutputFolder = traceOutputFolder
    # # Parse the traceOutputFolder path
    # while "(" in temptraceOutputFolder and not r"\(" in temptraceOutputFolder:
    #     temptraceOutputFolder = temptraceOutputFolder[:temptraceOutputFolder.find(
    #         "(")] + r'\(' + temptraceOutputFolder[temptraceOutputFolder.find("(") + 1:]
    # while ")" in temptraceOutputFolder and not r"\)" in temptraceOutputFolder:
    #     temptraceOutputFolder = temptraceOutputFolder[:temptraceOutputFolder.find(
    #         ")")] + r'\)' + temptraceOutputFolder[temptraceOutputFolder.find(")") + 1:]

    trace_files = [f for f in os.listdir(currentpath)
                   if f.endswith(".txt") and f.startswith("llfi.stat.trace.")]
    if not trace_files:
        print("Cannot find Trace input files.")
        print("Please make sure you are running this script in the "
              "llfi_stat_output folder")
        raise RuntimeError()
    else:
        with mp.Pool() as pool:
            print('Processing {} trace files...'.format(len(trace_files)),
                  flush=True)
            results = pool.imap_unordered(call_trace_diff, trace_files)
            errs = 0
            with open(log_path, 'w') as log_file:
                for trace_file, exitcode, out in results:
                    if exitcode != 0:
                        errs += 1
                    print('LOG: {} : exit={}\n'
                          '==================================='.format(
                              trace_file, exitcode), file=log_file)
                    print(out, file=log_file)
                    print('===================================\n',
                          file=log_file)
            print('Processed {} traces. {} had errors.'.format(
                len(trace_files), errs), flush=True)


def call_trace_diff(trace_file):
    print('Processing: {}'.format(trace_file), flush=True)
    cmd = (scriptdir + "/tracediff " + '--quick ' if QUICK_MODE else '' +
           goldenTraceFilePath + " " +
           trace_file + " > " + traceOutputFolder + "/TraceDiffReportFile" +
           trace_file[trace_file.find("llfi.stat.trace") +
                      len("llfi.stat.trace"):])
    try:
        exitcode = 0
        out = subprocess.check_output(cmd, shell=True,
                                      universal_newlines=True,
                                      stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as err:
        exitcode = err.returncode
        out = err.output
    return (trace_file, exitcode, out)


def generateDotFile():
    log_path = os.path.abspath(
        os.path.join(traceOutputFolder, 'generate-dot.log'))
    goldenTraceDotFile = os.path.abspath(os.path.join(
        currentpath, "../../../llfi.stat.graph.dot"))
    if not os.path.isfile(goldenTraceDotFile):
        goldenTraceDotFile = os.path.abspath(
            os.path.join(currentpath, "../../llfi.stat.graph.dot"))
        if not os.path.isfile(goldenTraceDotFile):
            print("Cannot find golden Trace Dot File 'llfi.stat.graph.dot'")
            raise RuntimeError()

    # # Parse the goldenTraceFile path
    # tempgoldenTraceFilePath = goldenTraceFilePath
    # while "(" in tempgoldenTraceFilePath and not r"\(" in tempgoldenTraceFilePath:
    #     tempgoldenTraceFilePath = tempgoldenTraceFilePath[:tempgoldenTraceFilePath.find(
    #         "(")] + r'\(' + tempgoldenTraceFilePath[tempgoldenTraceFilePath.find("(") + 1:]
    # while ")" in tempgoldenTraceFilePath and not r"\)" in tempgoldenTraceFilePath:
    #     tempgoldenTraceFilePath = tempgoldenTraceFilePath[:tempgoldenTraceFilePath.find(
    #         ")")] + r'\)' + tempgoldenTraceFilePath[tempgoldenTraceFilePath.find(")") + 1:]
    # tempScriptdir = scriptdir
    # # Parse the scriptdir path
    # while "(" in tempScriptdir and not r"\(" in tempScriptdir:
    #     tempScriptdir = tempScriptdir[:tempScriptdir.find(
    #         "(")] + r'\(' + tempScriptdir[tempScriptdir.find("(") + 1:]
    # while ")" in tempScriptdir and not r"\)" in tempScriptdir:
    #     tempScriptdir = tempScriptdir[:tempScriptdir.find(
    #         ")")] + r'\)' + tempScriptdir[tempScriptdir.find(")") + 1:]
    # temptraceOutputFolder = traceOutputFolder
    # # Parse the traceOutputFolder path
    # while "(" in temptraceOutputFolder and not r"\(" in temptraceOutputFolder:
    #     temptraceOutputFolder = temptraceOutputFolder[:temptraceOutputFolder.find(
    #         "(")] + r'\(' + temptraceOutputFolder[temptraceOutputFolder.find("(") + 1:]
    # while ")" in temptraceOutputFolder and not r"\)" in temptraceOutputFolder:
    #     temptraceOutputFolder = temptraceOutputFolder[:temptraceOutputFolder.find(
    #         ")")] + r'\)' + temptraceOutputFolder[temptraceOutputFolder.find(")") + 1:]
    # tempgoldenTraceDotFile = goldenTraceDotFile
    # # Parse the traceOutputFolder path
    # while "(" in tempgoldenTraceDotFile and not r"\(" in tempgoldenTraceDotFile:
    #     tempgoldenTraceDotFile = tempgoldenTraceDotFile[:tempgoldenTraceDotFile.find(
    #         "(")] + r'\(' + tempgoldenTraceDotFile[tempgoldenTraceDotFile.find("(") + 1:]
    # while ")" in tempgoldenTraceDotFile and not r"\)" in tempgoldenTraceDotFile:
    #     tempgoldenTraceDotFile = tempgoldenTraceDotFile[:tempgoldenTraceDotFile.find(
    #         ")")] + r'\)' + tempgoldenTraceDotFile[tempgoldenTraceDotFile.find(")") + 1:]

    diff_files = [f for f in os.listdir(traceOutputFolder)
                  if f.startswith("TraceDiffReportFile")]
    if not diff_files:
        print("Cannot find Trace diff reports.")
        print("Please make sure you are running this script in the "
              "llfi_stat_output folder")
        raise RuntimeError()
    else:
        with mp.Pool() as pool:
            print('Processing {} diff reports...'.format(len(diff_files)),
                  flush=True)
            results = pool.imap_unordered(
                call_gen_dot, zip(diff_files, it.repeat(goldenTraceDotFile)))
            errs = 0
            with open(log_path, 'w') as log_file:
                for diff_file, exitcode, out in results:
                    if exitcode != 0:
                        errs += 1
                    print('LOG: {} : exit={}\n'
                          '==================================='.format(
                              diff_file, exitcode), file=log_file)
                    print(out, file=log_file)
                    print('===================================\n',
                          file=log_file)
            print('Processed {} reports. {} had errors.'.format(
                len(diff_files), errs), flush=True)


def call_gen_dot(args):
    diff_file, goldenTraceDotFile = args
    print('Processing: {}'.format(diff_file), flush=True)
    name = diff_file[diff_file.find("TraceDiffReportFile") +
                     len("TraceDiffReportFile"):]
    name = name.replace(".txt", ".dot")
    cmd = (scriptdir + "/traceontograph " + traceOutputFolder + "/" +
           diff_file + " " + goldenTraceDotFile + " > " +
           traceOutputFolder + "/TraceGraph" + name)
    try:
        exitcode = 0
        out = subprocess.check_output(cmd, shell=True,
                                      universal_newlines=True,
                                      stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as err:
        exitcode = err.returncode
        out = err.output
    return (diff_file, exitcode, out)


def main():
    args = parseArgs()
    findPath()
    makeTraceOutputFolder()
    executeTraceDiff()
    if not args.no_dot:
        generateDotFile()
    else:
        print('Skipping dot generation by user request.')


if __name__ == "__main__":
    main()
