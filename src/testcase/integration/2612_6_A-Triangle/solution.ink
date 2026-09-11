// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_array(4);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      read(a[i]);
      i += 1;
    }
  }
  var flag: dynamic = false;
  if (((((a[0] + a[1]) > a[2]) && ((a[0] + a[2]) > a[1])) && ((a[1] + a[2]) > a[0])))
  {
    write("TRIANGLE");
    return 0;
  } else
  {
    if (((((a[0] + a[1]) == a[2]) || ((a[1] + a[2]) == a[0])) || ((a[0] + a[2]) == a[1])))
    {
      flag = true;
    }
  }
  if (((((a[0] + a[1]) > a[3]) && ((a[0] + a[3]) > a[1])) && ((a[3] + a[1]) > a[0])))
  {
    write("TRIANGLE");
    return 0;
  } else
  {
    if (((((a[0] + a[1]) == a[3]) || ((a[3] + a[1]) == a[0])) || ((a[0] + a[3]) == a[1])))
    {
      flag = true;
    }
  }
  if (((((a[0] + a[2]) > a[3]) && ((a[0] + a[3]) > a[2])) && ((a[3] + a[2]) > a[0])))
  {
    write("TRIANGLE");
    return 0;
  } else
  {
    if (((((a[0] + a[2]) == a[3]) || ((a[3] + a[2]) == a[0])) || ((a[0] + a[3]) == a[2])))
    {
      flag = true;
    }
  }
  if (((((a[1] + a[2]) > a[3]) && ((a[1] + a[3]) > a[2])) && ((a[3] + a[2]) > a[1])))
  {
    write("TRIANGLE");
    return 0;
  } else
  {
    if (((((a[1] + a[2]) == a[3]) || ((a[1] + a[3]) == a[2])) || ((a[3] + a[2]) == a[1])))
    {
      flag = true;
    }
  }
  if (flag)
  {
    write("SEGMENT");
  } else
  {
    write("IMPOSSIBLE");
  }
  return 0;
}
