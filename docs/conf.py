# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

# -- Project information -----------------------------------------------------
import subprocess
from datetime import date

project = 'Embedded SGP'
copyright = '{}, Sensirion AG'.format(date.today().year)


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'breathe'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
html_logo = 'sensirion.jpg'
html_theme_options = {
    'logo_only': True
}

# -- Options for PDF output -------------------------------------------------

latex_engine = 'pdflatex'

# latex_documents = [(startdocname, targetname, title, author, documentclass, toctree_only), ...].
# See sphinx docs for more information.
latex_documents = [
    (
        'sgp40/index',
        'Application_Note_SGP40.tex',
        'SGP40 Driver Integration',
        'Sensirion AG',
        'howto',
        True
    ),
    (
        'sgp40_voc_index/index',
        'Application_Note_SGP40_VOC_Index_Driver.tex',
        'SGP40 VOC Index Driver Integration',
        'Sensirion AG',
        'howto',
        True
    ),
    (
        'sgp40_voc_index/arduino_index',
        'Application_Note_SGP40_VOC_Index_Driver_Arduino.tex',
        'SGP40 VOC Index Driver Arduino Integration',
        'Sensirion AG',
        'howto',
        True
    ),
]
latex_logo = html_logo

# We could include this preamble from a .tex file. The problem is as, you'll see in the latex_elements dictionary,
# that some packages, like for example geometry, need to be declared separately. Otherwise they get imported by
# sphinx and additionally by our template, which is not allowed by latex. As far as I can tell, there is currently no
# option to tell sphinx it should leave the preamble entirely up to the user. I decided to keep it all inside conf.py
# and not import from a .tex file, because that only splits the latex settings over two files.
latex_preamble = r'''
    % page style
    \usepackage{lastpage}
    \makeatletter
    
    % \fancypagestyle{normal} has to be overwritten for styling because this is what is being used by sphinx.sty
    \fancypagestyle{normal} {
        \pagestyle{fancy}
        \lhead{\includegraphics[height=0.8cm]{sensirion.jpg}}
        \chead{}
        \rhead{\@date{}}
        \lfoot{\@title{}}
        \cfoot{}
        \rfoot{\thepage\ / \pageref{LastPage}}
        \renewcommand{\headrulewidth}{0.4pt}
        \renewcommand{\footrulewidth}{0.4pt}
    }
    
    % environment for placing stuff at bottom of titlepage    
    \newenvironment{bottompar}{\par\vspace*{\fill}}{\clearpage} % place at bottom of page
    \makeatother
'''

latex_titlepage = r'''
    \makeatletter
    \begin{titlepage}
    
        \begingroup % for PDF information dictionary
            \def\endgraf{ }\def\and{\& }%
            \pdfstringdefDisableCommands{\def\\{, }}% overwrite hyperref setup
            \hypersetup{pdfauthor={\@author}, pdftitle={\@title}, pdfkeywords={\py@release}}%
        \endgroup
    
        \centering \vspace*{2cm}
        \includegraphics[width=7cm]{sensirion.jpg}\par
        \vspace{2cm}
        {\huge\bfseries\@title{}\par}
        \vspace{2.5cm}
        {\Large\@author{}\par}
        \vspace{1cm}
        {\large\@date{}\par}
        
        \begin{bottompar}
            \small\py@release
        \end{bottompar}
    \end{titlepage}
    \makeatother
'''

result = subprocess.run(['git', 'describe', '--dirty', '--always'], stdout=subprocess.PIPE)
release_name = result.stdout.decode('utf-8').strip()

latex_elements = {
    'geometry': r'\usepackage[verbose,tmargin=2.5cm,bmargin=2.5cm,lmargin=2.5cm,rmargin=2.5cm]{geometry}',
    'releasename': r'document version: {}'.format(release_name),
    'preamble': latex_preamble,
    'maketitle': latex_titlepage
}

# -- Options for breathe extension -------------------------------------------------

breathe_projects = {
    "embedded_sgp": '_doxy_build/xml'
}
breathe_default_project = "embedded_sgp"
