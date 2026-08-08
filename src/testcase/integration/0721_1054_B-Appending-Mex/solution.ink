// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic)
{
  write(name, " : ", arg1, "\n");
}

func f(names: dynamic, arg1: dynamic, args: dynamic...)
{
  var comma = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << " : ") << arg1) << " | ");
  f((comma + 1), cpp_expand(args));
}

func XpowerY(x: dynamic, y: dynamic, m: dynamic)
{
  var ans = 1;
  x = (x % m);
  while ((y > 0))
  {
    if (((y % 2) == 1))
    {
      ans = (((ans * x)) % m);
    }
    x = (((((x % m)) * ((x % m)))) % m);
    y = (y >> 1);
  }
  return (ans % m);
}

var st: dynamic;

func __cpp_top_level_1()
{
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      if ((i == 1))
      {
        if ((x != 0))
        {
          write(1, "\n");
          return 0;
        } else
        {
          st.insert(0);
        }
        i += 1;
        continue;
      }
      if (((cpp_binary((st.size() == x), "and", ((*st.rbegin()) == (x - 1)))) || (st.find(x) != st.end())))
      {
        st.insert(x);
        i += 1;
        continue;
      }
      write(i, "\n");
      return 0;
      i += 1;
    }
  }
  write(-1, "\n");
  return 0;
}
