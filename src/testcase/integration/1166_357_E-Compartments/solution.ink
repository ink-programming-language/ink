// Translated from solution.cpp.

var ans: dynamic;

var a = cpp_array(10);

var flag = true;

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      var ad: dynamic;
      read(ad);
      a[ad] += 1;
      i += 1;
    }
  }
  ans += min(a[1], a[2]);
  a[3] += min(a[1], a[2]);
  var ad = min(a[1], a[2]);
  a[1] -= ad;
  a[2] -= ad;
  if (a[1])
  {
    ad = (((a[1] / 3)) * 2);
    ans += ad;
    a[3] += (a[1] / 3);
    a[1] %= 3;
    if (((a[1] == 1) && a[3]))
    {
      ans += 1;
    } else if (((a[1] == 1) && (a[4] >= 2)))
    {
      ans += 2;
    } else if ((a[1] == 1))
    {
      flag = false;
    }
    if (((a[1] == 2) && (a[3] >= a[1])))
    {
      ans += a[1];
    } else if (((a[1] == 2) && a[4]))
    {
      ans += 2;
    } else if ((a[1] == 2))
    {
      flag = false;
    }
  } else if (a[2])
  {
    ad = ((a[2] / 3) * 2);
    ans += ad;
    a[2] %= 3;
    a[3] += ad;
    if ((a[2] == 2))
    {
      ans += 2;
    } else if (((a[2] == 1) && a[4]))
    {
      ans += 1;
    } else if (((a[2] == 1) && (a[3] >= 2)))
    {
      ans += 2;
    } else if (a[2])
    {
      flag = false;
    }
  }
  if (flag)
  {
    write(ans, "\n");
  } else
  {
    write(-1);
  }
  return 0;
}
