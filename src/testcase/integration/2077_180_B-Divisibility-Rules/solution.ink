// Translated from solution.cpp.

func t2(a: dynamic, b: dynamic) -> dynamic
{
  var k: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < int_cpp(100)))
    {
      k *= a;
      if (((k % b) == 0))
      {
        return (i + 1);
      }
      k %= b;
      i += 1;
    }
  }
  return 0;
}

func t3(a: dynamic, b: dynamic) -> dynamic
{
  if ((((a % b)) == 1))
  {
    return true;
  }
  return false;
}

func t11(a: dynamic, b: dynamic) -> dynamic
{
  if ((((a % b) == 1) && (((a * a) % b) == (b - 1))))
  {
    return true;
  }
  if ((((a % b) == (b - 1)) && (((a * a) % b) == 1)))
  {
    return true;
  }
  return false;
}

func t6(a: dynamic, b: dynamic) -> dynamic
{
  var a2: dynamic = 0;
  var a3: dynamic = 0;
  var a11: dynamic = 0;
  {
    var i: dynamic = 2;
    while (((i * i) <= b))
    {
      if (((b % i) == 0))
      {
        var perm: dynamic = 1;
        while (((b % i) == 0))
        {
          perm *= i;
          b /= i;
        }
        var ok: dynamic = false;
        if (t2(a, perm))
        {
          ok = cpp_assign(a2, "=", 1);
        }
        if (t3(a, perm))
        {
          ok = cpp_assign(a3, "=", 1);
        }
        if (t11(a, perm))
        {
          ok = cpp_assign(a11, "=", 1);
        }
        if ((!ok))
        {
          return false;
        }
      }
      i += 1;
    }
  }
  if ((b > 1))
  {
    var ok: dynamic = false;
    var perm: dynamic = b;
    if (t2(a, perm))
    {
      ok = cpp_assign(a2, "=", 1);
    }
    if (t3(a, perm))
    {
      ok = cpp_assign(a3, "=", 1);
    }
    if (t11(a, perm))
    {
      ok = cpp_assign(a11, "=", 1);
    }
    if ((!ok))
    {
      return false;
    }
  }
  return (((a2 + a3) + a11) >= 2);
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  if (t2(a, b))
  {
    write("2-type", "\n", t2(a, b), "\n");
  } else if (t3(a, b))
  {
    write("3-type", "\n");
  } else if (t11(a, b))
  {
    write("11-type", "\n");
  } else if (t6(a, b))
  {
    write("6-type", "\n");
  } else
  {
    write("7-type", "\n");
  }
  return 0;
}
