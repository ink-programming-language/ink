// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func search(num: dynamic, a: dynamic) -> dynamic
{
  var cpp_ref: dynamic = a[num];
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= 3))
    {
      a[num] = i;
      var ind1: dynamic = num;
      var ind2: dynamic = num;
      var ind1_p: dynamic = ind1;
      var ind2_p: dynamic = ind2;
      while (true)
      {
        if (((ind1 < 0) || (ind2 > (n - 1))))
        {
          break;
        }
        if ((a[ind1] != a[ind2]))
        {
          break;
        }
        var cnt: dynamic = 2;
        if ((ind1 == ind2))
        {
          cnt -= 1;
        }
        while (((ind1 > 0) && (a[ind1] == a[(ind1 - 1)])))
        {
          ind1 -= 1;
          cnt += 1;
        }
        while (((ind2 < (n - 1)) && (a[ind2] == a[(ind2 + 1)])))
        {
          ind2 += 1;
          cnt += 1;
        }
        if ((cnt < 4))
        {
          break;
        }
        ind1_p = ind1;
        ind2_p = ind2;
        ind1 -= 1;
        ind2 += 1;
      }
      ans = max(ans, ((ind2_p - ind1_p) + 1));
      i += 1;
    }
  }
  a[num] = cpp_ref;
  if ((ans < 4))
  {
    ans = 0;
  }
  return ans;
}

func main() -> dynamic
{
  var a: dynamic = cpp_array(20000);
  while (true)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        ans = max(ans, search(i, a));
        i += 1;
      }
    }
    write((n - ans), "\n");
  }
  return 0;
}
