// Translated from solution.cpp.

func check() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  read(a, b);
  var v: dynamic = cpp_uninitialized();
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
      var c: dynamic = v[i];
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

func main() -> dynamic
{
  var ifile: dynamic = cpp_construct("input.txt");
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
