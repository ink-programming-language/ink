// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var t: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_construct((N - 1));
  var C: dynamic = cpp_construct((N - 1));
  read(t);
  var a: dynamic = t;
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      var b: dynamic = cpp_uninitialized();
      read(b);
      T[i] = abs((b - a));
      a = b;
      i += 1;
    }
  }
  read(c);
  var w: dynamic = c;
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      var b: dynamic = cpp_uninitialized();
      read(b);
      C[i] = abs((w - b));
      w = b;
      i += 1;
    }
  }
  if (((c != t) || (a != w)))
  {
    write("No");
  } else
  {
    sort(T.begin(), T.end());
    sort(C.begin(), C.end());
    {
      var i: dynamic = 0;
      while ((i < (N - 1)))
      {
        if ((C[i] != T[i]))
        {
          write("No");
          return 0;
        }
        i += 1;
      }
    }
    write("Yes");
  }
  return 0;
}
