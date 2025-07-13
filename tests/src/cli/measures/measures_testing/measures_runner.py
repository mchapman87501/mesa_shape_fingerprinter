"""
Provides a measures utility runner, one which returns status, stdout, stderr.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import os
import logging
import subprocess


class Error(Exception):
    pass


logger = logging.getLogger("MR")
logger.setLevel(logging.ERROR)


class MeasuresRunner:
    """Runs a measures utility, returning it exit status, stdout, stderr.
    Example:
    >>> import fp_file_generator
    >>> import measure
    >>> import fp_measurer
    >>> import expected_results_generator
    >>> import results_verifier
    >>> import results_reader
    >>> import os
    >>> default_exe = "../../measures_all"

    >>> fp_gen = fp_file_generator.FPFileGenerator(5)

    >>> measure = measure.Tani()
    >>> measurer = fp_measurer.get_measurer(measure, 5, True)
    >>> erg = expected_results_generator.Matrix(measurer, None, None)

    >>> runner = MeasuresRunner(fp_gen, erg, default_exe)
    >>> status, out, err = runner.run()
    >>> if status:
    ...     print("Error output: {0}".format(err))
    >>> status
    0

    """

    def __init__(
        self, file_generator, expecter, default_exe=None, get_arglist=None
    ):
        self._file_gen = file_generator
        self._expecter = expecter
        self._exe = os.environ.get("EXE", default_exe)
        if (self._exe is None) or not os.path.exists(self._exe):
            raise Error(
                "MeasuresRunner cannot find measures executable ({0})".format(
                    self._exe
                )
            )
        self._get_arglist = get_arglist or self._default_get_arglist

    def run(self):
        with self._file_gen as _fg:
            args = self._get_arglist(self._exe, self._file_gen, self._expecter)
            logger.debug(" ".join([repr(s) for s in args]))
            completion = subprocess.run(
                args, capture_output=True, encoding="utf8"
            )
            status = completion.returncode
            if status:
                logger.error(f"MeasuresRunner.run: Status {status} for {args}")
            return completion

    def _default_get_arglist(self, exe, file_gen, expecter):
        measurer = expecter.fp_measurer()
        result = [
            str(exe),
            file_gen.pathname(),
            measurer.measure_opt(),
            measurer.similarity_opt(),
            expecter.format_opt(),
            expecter.searching_opt(),
            str(expecter.search_number() or "0"),
        ]
        alpha = measurer.tversky_alpha()
        if alpha is not None:
            result.append(str(alpha))
        thresh = expecter.threshold()
        if thresh is not None:
            result.append(str(thresh))
        return result


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
