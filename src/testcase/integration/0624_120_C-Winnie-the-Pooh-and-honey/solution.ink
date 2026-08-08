// Translated from solution.cpp.

func check()
{
  var a: dynamic;
  var b: dynamic;
  var s = 0;
  var k: dynamic;
  read(a, b);
  var v: dynamic;
  {
    typeof(a) = 0;
    while ((i < (a)))
    {
      read(k);
      v.push_back(k);
      i += 1;
    }
  }
  {
    typeof(a) = 0;
    while ((i < (a)))
    {
      var c = v[i];
      {
        typeof(3) = 0;
        while ((j < (3)))
        {
          if (((c - b) >= 0))
          {
            c -= b;
          }
          j += 1;
        }
      }
      s += c;
      i += 1;
    }
  }
  write("\n", s);
}

func main()
{
  var ifile = cpp_construct("input.txt");
  if (ifile)
  {
    freopen("input.txt", "rt", stdin);
  }
  if (ifile)
  {
    freopen("output.txt", "wt", stdout);
  }
  check();
  return 0;
}
