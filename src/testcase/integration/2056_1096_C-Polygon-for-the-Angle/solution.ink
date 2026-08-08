// Translated from solution.cpp.

func shaon()
{
  write("\n");
}

func shaon(arg: dynamic, rest: dynamic...)
{
  write(arg, cpp_char(" "));
  shaon(cpp_expand(rest));
}

var eps = 1e-10;

func equalTo(a: dynamic, b: dynamic)
{
  if ((abs((a - b)) <= eps))
  {
    return true;
  } else
  {
    return false;
  }
}

func notEqual(a: dynamic, b: dynamic)
{
  if ((abs((a - b)) > eps))
  {
    return true;
  } else
  {
    return false;
  }
}

func lessThan(a: dynamic, b: dynamic)
{
  if (((a + eps) < b))
  {
    return true;
  } else
  {
    return false;
  }
}

func lessThanEqual(a: dynamic, b: dynamic)
{
  if ((a < (b + eps)))
  {
    return true;
  } else
  {
    return false;
  }
}

func greaterThan(a: dynamic, b: dynamic)
{
  if ((a > (b + eps)))
  {
    return true;
  } else
  {
    return false;
  }
}

func greaterThanEqual(a: dynamic, b: dynamic)
{
  if (((a + eps) > b))
  {
    return true;
  } else
  {
    return false;
  }
}

var t: dynamic;

var x: dynamic;

var arr = cpp_array(300002);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  {
    var i = 3;
    while ((i <= 300000))
    {
      var k = (((180.0 / i) * 1.0));
      arr[i] = k;
      i += 1;
    }
  }
  read(t);
  {
    var i = 1;
    while ((i <= t))
    {
      read(x);
      var bo = 1;
      {
        var w = 3;
        while ((w <= 300000))
        {
          var p = (x / arr[w]);
          var pk = ((arr[w] * p) * 1.0);
          if (equalTo(pk, x))
          {
            if ((p <= ((w - 2))))
            {
              bo = 0;
              write(cpp_cast(w), "\n");
              break;
            }
          }
          w += 1;
        }
      }
      if (bo)
      {
        write("-1\n");
      }
      i += 1;
    }
  }
  return 0;
}
