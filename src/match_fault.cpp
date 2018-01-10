/*
 * ansvif
 * A Not So Very Intelligent Fuzzer
 * Marshall Whittaker / oxagast
 */

//    __ _  _  __   ___  __  ____ ____
//   /  ( \/ )/ _\ / __)/ _\/ ___(_  _)
//  (  O )  (/    ( (_ /    \___ \ )(
//   \__(_/\_\_/\_/\___\_/\_(____/(__)

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "../include/xmlwriter/xml_writer.hpp"
#ifdef __linux
#ifdef __REDHAT
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#endif
#ifdef __DEBIAN
#include <crypto++/hex.h>
#include <crypto++/md5.h>
#endif
#endif
#ifdef __unix
#include "src/version.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

void log_hang(std::string write_file_n, std::string out_str_p,
              std::string out_str, std::string junk_file_of_args, int pid);
void log_tail(std::string write_file_n, std::string junk_file_of_args,
              std::string output_logfile, std::string crash_logfile,
              std::string cmd_output, std::string out_str_p,
              std::string out_str, int pid);
void log_head(std::string write_file_n, std::string path_str,
              std::string cmd_output, std::string out_str_p,
              std::string out_str, int pid);
std::string remove_chars(const std::string &source, const std::string &chars);
int reaper(int grim, int t_timeout);
FILE *popen2(std::string command, std::string type, int &pid,
             std::string low_lvl_user);
int pclose2(FILE *fp, pid_t pid);
void write_seg(std::string filename, std::string seg_line);
int rand_me_plz(int rand_from, int rand_to);
std::string make_garbage(int trash, int buf, std::string opt_other_str,
                         bool is_other, bool never_rand);
bool file_exists(const std::string &filen);
void write_junk_file(std::string filename, std::vector<std::string> opt_other,
                     int buf_size, int rand_spec_one, int rand_spec_two,
                     bool never_rand, std::string other_sep, bool verbose);
std::vector<std::string>
get_out_str(std::string env_str, std::string valgrind_str, std::string sys_str,
            std::string path_str, std::string always_arg_before,
            std::string always_arg_after, std::string fuzz_after,
            std::string log_prefix, std::string before_command);
std::vector<std::string>
get_out_str_pc(std::string env_str, std::string valgrind_str,
               std::string sys_str, std::string path_str,
               std::string always_arg_before, std::string always_arg_after,
               std::string fuzz_after, std::string log_prefix,
               std::string before_command);
bool match_seg(int buf_size, std::vector<std::string> opts,
               std::vector<std::string> spec_env, std::string path_str,
               std::string strip_shell, bool rand_all, bool write_to_file,
               std::string write_file_n, bool rand_buf,
               std::vector<std::string> opt_other, bool is_other,
               std::string other_sep, int t_timeout, std::string low_lvl_user,
               std::string junk_file_of_args, std::string always_arg_before,
               std::string always_arg_after, bool never_rand,
               std::string run_command, std::string fault_code, bool valgrind,
               bool single_try, bool percent_sign, int static_args,
               bool keep_going, std::string before_command, bool verbose,
               bool debug) {
  bool segged = false;
  std::vector<std::string> used_token;
  std::string valgrind_str;
  std::string output_logfile = write_file_n + ".output.ansvif.log";
  std::string crash_logfile = write_file_n + ".crash.ansvif.log";
  std::string valgrind_logfile = write_file_n + ".valgrind.ansvif.log";
  if (valgrind == true) {
    if (write_file_n != "") {
      /* if valgrind is active then we'll tel lit to use
       * an error code (139) as a fault code so that we
       * can trap it, it will also be logging if we are
       */
      valgrind_str = "/usr/bin/valgrind --leak-check=full --xml=yes "
                     "--xml-file=" +
                     write_file_n + ".valgrind.ansvif.log --error-exitcode=139";
    } else {
      /* also for the valgrind wrapper but this time we're
       * not logging
       */
      valgrind_str = "/usr/bin/valgrind --leak-check=full --error-exitcode=139";
    }
  }
  if (valgrind == false) {
    /* not using valgrind */
    valgrind_str = "";
  }
  if (file_exists(path_str) == true) {
    /* check to make sure the file we're going to run
     * is actually an executable file
     */
    while (segged == false) {
      unlink(crash_logfile.c_str());
      unlink(output_logfile.c_str());
      unlink(valgrind_logfile.c_str());
      /* initialize two random pieces */
      int rand_spec_one, rand_spec_two;
      std::vector<std::string> junk_opters;

      if (rand_all == true) {
        /* or always random data (8) */
        rand_spec_one = 8;
        rand_spec_two = 8;
      } else {
        /* any data entered */
        rand_spec_one = 0;
        rand_spec_two = 19;
      }
      /* initilize our optiosn vectors, our environment string
       * and our system call string
       */
      std::vector<std::string> junk_opts_env, junk_opts;
      std::string env_str, sys_str;
      if (junk_file_of_args != "") {
        /* write a junk file */
        write_junk_file(junk_file_of_args, opt_other, buf_size, rand_spec_one,
                        rand_spec_two, never_rand, other_sep, verbose);
      }
      int sep_type;
      /* so we don't have warnings with -Wall */
      int opts_size = opts.size();
      int my_penis_size = spec_env.size();
      /* loop around the options, roll the die, and put a
       * random argument in the vector
       */
      if (static_args == 0) {
        for (int cmd_flag_l = 0; cmd_flag_l < opts_size; cmd_flag_l++) {
          if (rand_me_plz(0, 1) == 1) {
            junk_opts.push_back(opts.at(cmd_flag_l));
          }
        }
      }
      if (static_args != 0) {
        for (int cmd_flag_l = 0; cmd_flag_l < opts_size; cmd_flag_l++) {
          if ((rand_me_plz(0, 1) == 1) && (cmd_flag_l < opts_size)) {
            junk_opts.push_back(opts.at(cmd_flag_l));
          } else {
            junk_opts.push_back(" ");
          }
        }
        std::random_shuffle(junk_opts.begin(), junk_opts.end());
        for (int holder = 0; holder < static_args; holder++) {
          junk_opters.push_back(opts.at(rand_me_plz(0, opts.size() - 1)));
        }
      }
      junk_opts = junk_opters;
      for (int cmd_flag_a = 0; cmd_flag_a < my_penis_size; cmd_flag_a++) {
        if (rand_me_plz(0, 1) == 1) {
          junk_opts_env.push_back(spec_env.at(cmd_flag_a));
        }
      }
      if (is_other == true) {
        if (rand_buf == true) {
          for (std::vector<std::string>::const_iterator junk_opt_env =
                   junk_opts_env.begin();
               junk_opt_env != junk_opts_env.end(); ++junk_opt_env) {
            /* environment variable random shit */
            std::string oscar_env = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             rand_me_plz(1, buf_size),
                             opt_other.at(rand_me_plz(0, opt_other.size() - 1)),
                             is_other, never_rand),
                " ");
            if (oscar_env != "OOR") {
              /* making sure it's not out of range */
              env_str = env_str + *junk_opt_env + oscar_env + " ";
            }
          }

          for (std::vector<std::string>::const_iterator junk_opt =
                   junk_opts.begin();
               junk_opt != junk_opts.end(); ++junk_opt) {
            std::string oscar = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             rand_me_plz(1, buf_size),
                             opt_other.at(rand_me_plz(0, opt_other.size() - 1)),
                             is_other, never_rand),
                strip_shell);
            if (oscar != "OOR") {
              sep_type = rand_me_plz(0, 1);
              if (sep_type == 0) {
                sys_str = " " + sys_str + *junk_opt + " " + oscar + " ";
              }
              if (sep_type == 1) {
                sys_str =
                    sys_str + " " + *junk_opt + other_sep + oscar + other_sep;
              }
            }
          }
        } else if (rand_buf == false) {
          for (std::vector<std::string>::const_iterator junk_opt_env =
                   junk_opts_env.begin();
               junk_opt_env != junk_opts_env.end(); ++junk_opt_env) {
            std::string oscar_env = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             buf_size,
                             opt_other.at(rand_me_plz(0, opt_other.size() - 1)),
                             is_other, never_rand),
                " ");
            if (oscar_env != "OOR") {
              /* really really repetative */
              env_str = env_str + *junk_opt_env + oscar_env + " ";
            }
          }
          for (std::vector<std::string>::const_iterator junk_opt =
                   junk_opts.begin();
               junk_opt != junk_opts.end(); ++junk_opt) {
            std::string oscar = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             buf_size,
                             opt_other.at(rand_me_plz(0, opt_other.size() - 1)),
                             is_other, never_rand),
                strip_shell);
            if (oscar != "OOR") { // if not out of range
              /* here we randomize if we have a space or not */
              sep_type = rand_me_plz(0, 1);
              if (sep_type == 0) {
                /* space */
                sys_str = " " + sys_str + *junk_opt + " " + oscar + " ";
              }
              if (sep_type == 1) {
                /* some other seperator like a comma or w/e */
                sys_str = " " + sys_str + " " + *junk_opt + other_sep + oscar +
                          other_sep;
              }
            }
          }
        }
      }
      if (is_other == false) {
        if (rand_buf == true) {
          for (std::vector<std::string>::const_iterator junk_opt_env =
                   junk_opts_env.begin();
               junk_opt_env != junk_opts_env.end(); ++junk_opt_env) {
            /* loop through the vector of junk environment
             * variables
             */
            std::string oscar_env = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             rand_me_plz(1, buf_size), "", is_other,
                             never_rand),
                " ");
            if (oscar_env != "OOR") {
              env_str = env_str + *junk_opt_env + oscar_env + " ";
            }
          }
          for (std::vector<std::string>::const_iterator junk_opt =
                   junk_opts.begin();
               junk_opt != junk_opts.end(); ++junk_opt) {
            /* loop through the vector of junk options */
            std::string oscar = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             rand_me_plz(1, buf_size), "", is_other,
                             never_rand),
                strip_shell);
            if (oscar != "OOR") {
              sep_type = rand_me_plz(0, 1);
              if (sep_type == 0) {
                sys_str = " " + sys_str + *junk_opt + " " + oscar + " ";
              }
              if (sep_type == 1) {
                sys_str =
                    sys_str + " " + *junk_opt + other_sep + oscar + other_sep;
              }
            }
          }
        } else if (rand_buf == false) {
          for (std::vector<std::string>::const_iterator junk_opt_env =
                   junk_opts_env.begin();
               junk_opt_env != junk_opts_env.end(); ++junk_opt_env) {
            std::string oscar_env = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             buf_size, "", is_other, never_rand),
                " ");
            if (oscar_env != "OOR") {
              env_str = env_str + *junk_opt_env + oscar_env + " ";
            }
          }

          for (std::vector<std::string>::const_iterator junk_opt =
                   junk_opts.begin();
               junk_opt != junk_opts.end(); ++junk_opt) {

            std::string oscar = remove_chars(
                make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                             buf_size, "", is_other, never_rand),
                strip_shell);
            if (oscar != "OOR") {
              sep_type = rand_me_plz(0, 1);
              if (sep_type == 0) {
                sys_str = " " + sys_str + *junk_opt + " " + oscar + " ";
              }
              if (sep_type == 1) {
                sys_str =
                    sys_str + " " + *junk_opt + other_sep + oscar + other_sep;
              }
            }
          }
        }
      }
      std::string fuzz_after =
          remove_chars(make_garbage(rand_me_plz(rand_spec_one, rand_spec_two),
                                    buf_size, "", is_other, never_rand),
                       strip_shell);
      std::vector<std::string> out_all;
      if (fuzz_after == "OOR") {
        fuzz_after = "";
      }
      if (percent_sign == true) {
        /* all of the stuff except for echo $? is now
         * put together and ready to be sent out to the
         * subroutine, $? is done in sys_string.cpp
         */
        out_all = get_out_str_pc(env_str, valgrind_str, sys_str, path_str,
                                 always_arg_before, always_arg_after,
                                 fuzz_after, write_file_n, before_command);
      }
      if (percent_sign == false) {
        out_all = get_out_str(env_str, valgrind_str, sys_str, path_str,
                              always_arg_before, always_arg_after, fuzz_after,
                              write_file_n, before_command);
      }
      /* coming to the stuff from sys_string either
       * normal or printf output
       */
      std::string out_str = out_all[0];
      std::string out_str_p = out_all[1];
      /* claer the vector and shrink it */
      junk_opts.clear();
      junk_opts.shrink_to_fit();
      junk_opts_env.clear();
      junk_opts_env.shrink_to_fit();
#ifdef __linux
      /* Make sure that we don't go and do things
       * we've already tried over and over, this
       * speeds things up quite a bit
       */
      byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
      CryptoPP::Weak::MD5 hash;
      hash.CalculateDigest(digest, (const byte *)out_str.c_str(),
                           out_str.length());
      CryptoPP::HexEncoder encoder;
      std::string h_output;
      encoder.Attach(new CryptoPP::StringSink(h_output));
      encoder.Put(digest, sizeof(digest));
      encoder.MessageEnd();
      /* std::cout << h_output << std::endl; */
      if (std::find(used_token.begin(), used_token.end(), h_output) !=
          used_token.end()) {
        used_token.push_back(h_output);
} else {
       used_token.push_back(h_output);
#endif
#ifdef __linux
        if (buf_size == 0) {
          out_str = before_command + " " + path_str + " " + always_arg_before +
                    " " + always_arg_after;
          out_str_p = before_command + " " + path_str + " " +
                      always_arg_before + " " + always_arg_after;
          if (write_file_n == "") {
            /* incase we are logging don't leave a blank file */
           out_str = out_str + " >/dev/null 2>&1; echo $?";
  
        } else {
            /* get the signal here and log */

            out_str = out_str + " >" + write_file_n +
                      ".output.ansvif.log 2>&1; echo $?";
          }
        }

#endif
        if (debug == true) {
          /* write ALL the junk to STDOUT since we're in
           * debug mode
           */
          std::cout << out_str << std::endl
                    << out_str_p << std::endl
                    << std::endl;
        }
        /* this here takes care of the command that is run after
         * the fuzz
         */
	if (run_command != "") {
#ifdef _WIN32
		  run_command = "powershell " + run_command;
#endif
	  int run_com_pid;
	  FILE *fp = popen2(run_command, "r", run_com_pid, low_lvl_user);
          pclose2(fp, run_com_pid);
        }
        /* inititalize the child and open the child process fork
         * 4096 bytes should be enough to handle whatever pops out
         * when we go to match.  we match with stringstream to get
         * output and put its contense in 'output'
         */
        int pid;
        FILE *fp = popen2(out_all[0], "r", pid, low_lvl_user);
        char command_out[4096] = {0};
        std::stringstream output;
        while (read(fileno(fp), command_out, sizeof(command_out) - 1) != 0) {
          output << std::string(command_out);
          /* make sure we don't overflow */
          memset(&command_out, 0, sizeof(command_out));
        }
        /* close out the command cleanly */
        pclose2(fp, pid);
/* this takes care of killing off the child if it takes
 * too long
 */
#ifdef __unix__
        std::thread reaper_thread(reaper, pid, t_timeout);
        /* takes care of the reaper thread */
        reaper_thread.detach();
#endif
        /* our output will be stored here! */
        std::string cmd_output = output.str();
        if (write_file_n != "") {
          /* logging */
          log_head(write_file_n, path_str, cmd_output, out_str_p, out_str, pid);
        }
	/* std::cout << cmd_output << std::endl; */
        std::size_t found132 = cmd_output.find("CRASHCODE 132");
        std::size_t found134 = cmd_output.find("CRASHCODE 134");
        std::size_t found139 = cmd_output.find("CRASHCODE 139");
        std::size_t found135 = cmd_output.find("CRASHCODE 135");
        std::size_t found136 = cmd_output.find("CRASHCODE 136");
        std::size_t found159 = cmd_output.find("CRASHCODE 159");
        std::size_t found138 = cmd_output.find("CRASHCODE 138");
		std::size_t foundW1073741819 = cmd_output.find("-1073741819");
        std::size_t foundW1073740791 = cmd_output.find("-1073740791");
        std::size_t foundW1073741571 = cmd_output.find("-1073741571");
        std::size_t foundW532459699 = cmd_output.find("-532459699");
	/*
	 * Windows codes:
	 * -1073741819 -1073740791 -1073741571 -532459699
     * Unix codes:
	 * 132 134 139 135 136 159 138
	 */
        std::size_t foundother = cmd_output.find("CRASHCODE " + fault_code);
        if ((found132 != std::string::npos) ||
            (found134 != std::string::npos) ||
            (found139 != std::string::npos) ||
            (found135 != std::string::npos) ||
            (found136 != std::string::npos) ||
            (found159 != std::string::npos) ||
            (found138 != std::string::npos) ||
			(foundW1073741819 != std::string::npos) ||
			(foundW1073740791 != std::string::npos) ||
			(foundW1073741571 != std::string::npos) ||
			(foundW532459699 != std::string::npos) ||
            (foundother != std::string::npos)) {
          cmd_output = cmd_output.replace(0, 22, "");
#ifdef __unix__
          std::cout << "PID: " << pid << std::endl;
#endif
          std::cout << "Exit Code: " << cmd_output << std::endl;
          std::cout << "Crashed with command: " << std::endl
                    << out_str_p << std::endl;
          if (junk_file_of_args != "") {
            /* log the file data too */
            std::cout << "File data left in: " << junk_file_of_args
                      << std::endl;
          }
          if (write_to_file == true) {
            /* since we crashed we're going to finish writing to the
             * xml file
             */
            log_tail(write_file_n, junk_file_of_args, output_logfile,
                     crash_logfile, cmd_output, out_str_p, out_str, pid);
            /* then exit cleanly because we crashed it! Get it? :) */
            /* logging hangs */
          }
          if (keep_going == false) {
            return (false);
          }
#ifdef __linux
        }  // For the checksum algorithm under linux
#endif   
      }
      if (single_try == true) {
        /* do all that shit but only once! */
        if ((verbose == true) || (debug == true)) {
          std::cout << "No fault of mine!" << std::endl;
        }
        /* this non standard code 64 is to tell a wrapper that
         * we never hit a fault with the single run
         */
        exit(64);
      }
    }
  }
  return (true);
}
