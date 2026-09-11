// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(n, l, r);
  var f: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v1[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((((v[i] != v1[i])) && (((i < (l - 1)) || (i > (r - 1))))))
      {
        f = 0;
        break;
      }
      i += 1;
    }
  }
  if (f)
  {
    write("TRUTH");
  } else
  {
    write("LIE");
  }
  return 0;
}
