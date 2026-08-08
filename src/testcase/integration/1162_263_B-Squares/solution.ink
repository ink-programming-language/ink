// Translated from solution.cpp.

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "{");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "}");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (((os << it->first) << " = ") << it->second);
      it += 1;
    }
  }
  return (os << "]");
}

func faltu()
{
  write("\n");
}

func faltu(a: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func faltu(arg: dynamic, rest: dynamic...)
{
  write(arg, cpp_char(" "));
  faltu(cpp_expand(rest));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  if ((n < k))
  {
    write(-1, "\n");
  } else
  {
    write(a[(n - k)], " ", a[(n - k)], "\n");
  }
  return 0;
}
