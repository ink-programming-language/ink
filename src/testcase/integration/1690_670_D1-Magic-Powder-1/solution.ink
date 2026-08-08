// Translated from solution.cpp.

var dx4 = [0, 0, -1, 1];

var dy4 = [-1, 1, 0, 0];

func valid(r: dynamic, c: dynamic, x: dynamic, y: dynamic)
{
  if (((((x >= 1) && (x <= r)) && (y >= 1)) && (y <= c)))
  {
    return 1;
  }
  return 0;
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array((n + 2));
  var b = cpp_array((n + 2));
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(b[i]);
      i += 1;
    }
  }
  var ans = 0;
  var cnt = 1;
  while (true)
  {
    if ((cnt == ((n + 1))))
    {
      cnt = 1;
      ans += 1;
    }
    if ((a[cnt] <= b[cnt]))
    {
      b[cnt] -= a[cnt];
    } else
    {
      if ((k < ((a[cnt] - b[cnt]))))
      {
        break;
      } else
      {
        k -= ((a[cnt] - b[cnt]));
        b[cnt] = 0;
      }
    }
    cnt += 1;
  }
  write(ans, "\n");
  return 0;
}
