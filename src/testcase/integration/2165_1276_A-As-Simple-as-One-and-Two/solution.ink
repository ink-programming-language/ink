// Translated from solution.cpp.

var max_n = 200111;

var inf = 1000111222;

var s: dynamic;

var buf = cpp_array(max_n);

func read_str()
{
  scanf("%s", buf);
  return buf;
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    s = read_str();
    var ans: dynamic;
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (((i + 4) >= s.size()))
        {
          break;
        }
        if ((((((s[i] == cpp_char("t")) && (s[(i + 1)] == cpp_char("w"))) && (s[(i + 2)] == cpp_char("o"))) && (s[(i + 3)] == cpp_char("n"))) && (s[(i + 4)] == cpp_char("e"))))
        {
          s[(i + 2)] = cpp_char(".");
          ans.push_back((i + 2));
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (((i + 2) >= s.size()))
        {
          break;
        }
        if ((((s[i] == cpp_char("t")) && (s[(i + 1)] == cpp_char("w"))) && (s[(i + 2)] == cpp_char("o"))))
        {
          ans.push_back((i + 1));
          s[(i + 1)] = cpp_char(".");
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (((i + 2) >= s.size()))
        {
          break;
        }
        if ((((s[i] == cpp_char("o")) && (s[(i + 1)] == cpp_char("n"))) && (s[(i + 2)] == cpp_char("e"))))
        {
          ans.push_back((i + 1));
          s[(i + 1)] = cpp_char(".");
        }
        i += 1;
      }
    }
    write(ans.size(), "\n");
    for (var a in ans)
    {
      write((a + 1), cpp_char(" "));
    }
    write("\n");
  }
  return 0;
}
