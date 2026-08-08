// Translated from solution.cpp.

func substring(t: dynamic, F: dynamic, T: dynamic)
{
  return t.substr(F, (T - F));
}

func tostr(a: dynamic)
{
  var os = cpp_construct("");
  (os << a);
  return os.str();
}

func sqr(x: dynamic)
{
  return (x * x);
}

func LOG(N: dynamic, B: dynamic)
{
  return ((log10l(N)) / (log10l(B)));
}

func tochr(a: dynamic)
{
  return cpp_cast((((cpp_cast(cpp_char("0"))) + a)));
}

func CompareDouble(a: dynamic, b: dynamic)
{
  if ((a < (b - 1.0e-11)))
  {
    return -1;
  } else
  {
    return 1;
  }
  return 0;
}

func main()
{
  var n: dynamic;
  read(n);
  var a: dynamic;
  var b: dynamic;
  var mya: dynamic;
  var myb: dynamic;
  var my: dynamic;
  {
    var i = 0;
    var n = (n);
    while ((i < n))
    {
      var a1: dynamic;
      var b1: dynamic;
      read(a1, b1);
      a.push_back(a1);
      b.push_back(b1);
      mya[a1] += 1;
      if ((a1 != b1))
      {
        myb[b1] += 1;
      }
      my[a1] += 1;
      my[b1] += 1;
      i += 1;
    }
  }
  var it: dynamic;
  var itb: dynamic;
  var m = (1 << 20);
  {
    it = my.begin();
    while ((it != my.end()))
    {
      var a1 = mya[((*it)).first];
      var b1 = myb[((*it)).first];
      if (((a1 + b1) >= (((n + 1)) / 2)))
      {
        if ((a1 >= (((n + 1)) / 2)))
        {
          write("0", "\n");
          return 0;
        } else
        {
          if (((((((n + 1)) / 2) - a1)) < m))
          {
            m = (((((n + 1)) / 2) - a1));
          }
        }
      }
      it += 1;
    }
  }
  if ((m == (1 << 20)))
  {
    write("-1", "\n");
  } else
  {
    write(m, "\n");
  }
}
