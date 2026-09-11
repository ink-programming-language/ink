// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  return  ((y == 0)) ? x : gcd(y, (x % y));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var val: dynamic = cpp_uninitialized();
          scanf("%d", (&val));
          m[(-val)] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var it: dynamic = m.begin();
    while ((it != m.end()))
    {
      var cnt: dynamic = it->second;
      if ((cnt == 0))
      {
        it += 1;
        continue;
      }
      var val: dynamic = (-(it->first));
      var divCnt: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < a.size()))
        {
          if (((a[i] % val) == 0))
          {
            divCnt += 1;
          }
          i += 1;
        }
      }
      var num: dynamic = 1;
      while (((((2 * divCnt) * num) + (num * num)) < cnt))
      {
        num += 1;
      }
      {
        var i: dynamic = 0;
        while ((i < a.size()))
        {
          var g: dynamic = gcd(a[i], val);
          m[(-g)] -= (2 * num);
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < num))
        {
          a.push_back(val);
          i += 1;
        }
      }
      it += 1;
    }
  }
  for (var val: dynamic in a)
  {
    printf("%d ", val);
  }
  printf("\n");
}
