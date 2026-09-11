// Translated from solution.cpp.

func get_answer_direct(str: dynamic, ch: dynamic) -> dynamic
{
  var answer: dynamic = 0;
  var begin: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < str.size()))
    {
      if ((str[i] != str[begin]))
      {
        if ((str[(i - 1)] == ch))
        {
          answer = max(answer, (i - begin));
        }
        begin = i;
      }
      i += 1;
    }
  }
  if ((str[(str.size() - 1)] == ch))
  {
    answer = max(answer, (cpp_cast(str.size()) - begin));
  }
  return answer;
}

func get_answer(n: dynamic, strings: dynamic, ch: dynamic) -> dynamic
{
  if ((n == 1))
  {
    return get_answer_direct(strings[0], ch);
  } else
  {
    var all_ch: dynamic = true;
    {
      var i: dynamic = 0;
      while ((i < strings[(n - 1)].size()))
      {
        if ((strings[(n - 1)][i] != ch))
        {
          all_ch = false;
          break;
        }
        i += 1;
      }
    }
    if (all_ch)
    {
      var answer_prev: dynamic = get_answer((n - 1), strings, ch);
      return (answer_prev + (((answer_prev + 1)) * strings[(n - 1)].size()));
    } else
    {
      var char_exists: dynamic = cpp_new();
      {
        var i: dynamic = 0;
        while ((i < 26))
        {
          char_exists[i] = false;
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < (n - 1)))
        {
          {
            var j: dynamic = 0;
            while ((j < strings[i].size()))
            {
              char_exists[(strings[i][j] - cpp_char("a"))] = true;
              j += 1;
            }
          }
          i += 1;
        }
      }
      var answer: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < 26))
        {
          if ((!char_exists[i]))
          {
            i += 1;
            continue;
          }
          var str_cur: dynamic = ((strings[(n - 1)] + string_cpp(1, cpp_cast(((cpp_char("a") + i))))) + strings[(n - 1)]);
          answer = max(answer, get_answer_direct(str_cur, ch));
          i += 1;
        }
      }
      return answer;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var strings: dynamic = cpp_new();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(strings[i]);
      i += 1;
    }
  }
  var answer: dynamic = 1;
  {
    var ch: dynamic = cpp_char("a");
    while ((ch <= cpp_char("z")))
    {
      answer = max(answer, get_answer(n, strings, ch));
      ch += 1;
    }
  }
  write(answer);
  return 0;
}
