// Translated from solution.cpp.

var maxn: dynamic = 5001;

var x11: dynamic = cpp_array(10);

var y11: dynamic = cpp_array(10);

var x22: dynamic = cpp_array(10);

var y22: dynamic = cpp_array(10);

var deltax: dynamic = cpp_array(10);

var deltay: dynamic = cpp_array(10);

func main() -> dynamic
{
  var flag1: dynamic = 0;
  var flag2: dynamic = 0;
  var flag3: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      scanf("%lld %lld %lld %lld", (&x11[i]), (&y11[i]), (&x22[i]), (&y22[i]));
      if (((x11[i] == x22[i]) && (y11[i] == y22[i])))
      {
        flag3 = 1;
      }
      deltax[i] = (x11[i] - x22[i]);
      deltay[i] = (y11[i] - y22[i]);
      if ((deltax[i] == 0))
      {
        flag1 = 1;
      }
      if ((deltay[i] == 0))
      {
        flag2 = 1;
      }
      i += 1;
    }
  }
  if ((((!flag1) || (!flag2)) || flag3))
  {
    printf("NO\n");
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      if (((deltax[i] != 0) && (deltay[i] != 0)))
      {
        printf("NO\n");
        return 0;
      }
      i += 1;
    }
  }
  var vx: dynamic = cpp_uninitialized();
  var vy: dynamic = cpp_uninitialized();
  var mp1: dynamic = cpp_uninitialized();
  var mp2: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      vx.push_back(x11[i]);
      vx.push_back(x22[i]);
      vy.push_back(y11[i]);
      vy.push_back(y22[i]);
      mp1[x11[i]] += 1;
      mp1[x22[i]] += 1;
      mp2[y11[i]] += 1;
      mp2[y22[i]] += 1;
      i += 1;
    }
  }
  if (((mp1.size() != 2) || (mp2.size() != 2)))
  {
    printf("NO\n");
  } else
  {
    var tempflag: dynamic = 0;
    for (var it: dynamic in mp1)
    {
      if ((it.second != 4))
      {
        tempflag = 1;
      }
    }
    for (var it: dynamic in mp2)
    {
      if ((it.second != 4))
      {
        tempflag = 1;
      }
    }
    if ((!tempflag))
    {
      printf("YES\n");
    } else
    {
      printf("NO\n");
    }
  }
  return 0;
}
