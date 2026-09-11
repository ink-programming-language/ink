// Translated from solution.cpp.

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << "{");
  {
    var it: dynamic = v.begin();
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

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << "[");
  {
    var it: dynamic = v.begin();
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

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << "[");
  {
    var it: dynamic = v.begin();
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

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << "[");
  {
    var it: dynamic = v.begin();
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

func faltu() -> dynamic
{
  write("\n");
}

func faltu(a: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func faltu(arg: dynamic, rest: dynamic...) -> dynamic
{
  write(arg, cpp_char(" "));
  faltu(cpp_expand(rest));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
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
