// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var t: dynamic;
  var c: dynamic;
  var T = cpp_construct((N - 1));
  var C = cpp_construct((N - 1));
  read(t);
  var a = t;
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      var b: dynamic;
      read(b);
      T[i] = abs((b - a));
      a = b;
      i += 1;
    }
  }
  read(c);
  var w = c;
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      var b: dynamic;
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
      var i = 0;
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
