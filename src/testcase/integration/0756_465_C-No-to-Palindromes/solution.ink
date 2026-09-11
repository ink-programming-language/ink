// Translated from solution.cpp.

var p: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func build(in_cpp: dynamic, i: dynamic) -> dynamic
{
  var gg: dynamic = 0;
  {
    var v: dynamic = (in_cpp[i] + 1);
    while ((v <= p))
    {
      if ((((((i - 1) < 0) || (in_cpp[(i - 1)] != v))) && ((((i - 2) < 0) || (in_cpp[(i - 2)] != v)))))
      {
        gg = true;
        in_cpp[i] = v;
        break;
      }
      v += 1;
    }
  }
  if (gg)
  {
    var can: dynamic = true;
    {
      var j: dynamic = (i + 1);
      while ((j < n))
      {
        can = false;
        {
          var v: dynamic = cpp_char("a");
          while ((v <= p))
          {
            if ((((((j - 1) < 0) || (in_cpp[(j - 1)] != v))) && ((((j - 2) < 0) || (in_cpp[(j - 2)] != v)))))
            {
              can = true;
              in_cpp[j] = v;
              break;
            }
            v += 1;
          }
        }
        if ((!can))
        {
          break;
        }
        if (((j + 1) == n))
        {
          write(in_cpp, "\n");
          return 0;
        }
        j += 1;
      }
    }
    if (can)
    {
      write(in_cpp, "\n");
      return 0;
    }
  }
  return 1;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  read(n, p);
  p += (cpp_char("a") - 1);
  var in_cpp: dynamic = cpp_uninitialized();
  read(in_cpp);
  {
    var i: dynamic = n;
    while ((cpp_update(i, "--") > 0))
    {
      if ((!build(in_cpp, i)))
      {
        return 0;
      }
    }
  }
  write("NO", "\n");
}
