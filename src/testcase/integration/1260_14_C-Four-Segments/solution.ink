// Translated from solution.cpp.

var maxn = 5001;

var x11 = cpp_array(10);

var y11 = cpp_array(10);

var x22 = cpp_array(10);

var y22 = cpp_array(10);

var deltax = cpp_array(10);

var deltay = cpp_array(10);

func main()
{
  var flag1 = 0;
  var flag2 = 0;
  var flag3 = 0;
  {
    var i = 0;
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
    var i = 0;
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
  var vx: dynamic;
  var vy: dynamic;
  var mp1: dynamic;
  var mp2: dynamic;
  {
    var i = 0;
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
    var tempflag = 0;
    for (var it in mp1)
    {
      if ((it.second != 4))
      {
        tempflag = 1;
      }
    }
    for (var it in mp2)
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
