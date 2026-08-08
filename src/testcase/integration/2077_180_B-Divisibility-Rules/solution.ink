// Translated from solution.cpp.

func t2(a: dynamic, b: dynamic)
{
  var k = 1;
  {
    var i = 0;
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

func t3(a: dynamic, b: dynamic)
{
  if ((((a % b)) == 1))
  {
    return true;
  }
  return false;
}

func t11(a: dynamic, b: dynamic)
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

func t6(a: dynamic, b: dynamic)
{
  var a2 = 0;
  var a3 = 0;
  var a11 = 0;
  {
    var i = 2;
    while (((i * i) <= b))
    {
      if (((b % i) == 0))
      {
        var perm = 1;
        while (((b % i) == 0))
        {
          perm *= i;
          b /= i;
        }
        var ok = false;
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
    var ok = false;
    var perm = b;
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

func main()
{
  var a: dynamic;
  var b: dynamic;
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
