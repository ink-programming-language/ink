// Translated from solution.cpp.

var a: dynamic;

var m: dynamic;

func gcd(x: dynamic, y: dynamic)
{
  return if ((y == 0)) x else gcd(y, (x % y));
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          var val: dynamic;
          scanf("%d", (&val));
          m[(-val)] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var it = m.begin();
    while ((it != m.end()))
    {
      var cnt = it->second;
      if ((cnt == 0))
      {
        it += 1;
        continue;
      }
      var val = (-(it->first));
      var divCnt = 0;
      {
        var i = 0;
        while ((i < a.size()))
        {
          if (((a[i] % val) == 0))
          {
            divCnt += 1;
          }
          i += 1;
        }
      }
      var num = 1;
      while (((((2 * divCnt) * num) + (num * num)) < cnt))
      {
        num += 1;
      }
      {
        var i = 0;
        while ((i < a.size()))
        {
          var g = gcd(a[i], val);
          m[(-g)] -= (2 * num);
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < num))
        {
          a.push_back(val);
          i += 1;
        }
      }
      it += 1;
    }
  }
  for (var val in a)
  {
    printf("%d ", val);
  }
  printf("\n");
}
