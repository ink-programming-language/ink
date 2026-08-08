// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  var c = cpp_construct((m + 5), 0);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var l = 0;
  var r = 0;
  var res = 1;
  c[(v[0] - 1)] += 1;
  var maxC = 1;
  var maxI = (v[0] - 1);
  while ((r < n))
  {
    if ((c[(v[r] - 1)] > maxC))
    {
      maxC = c[(v[r] - 1)];
      maxI = (v[r] - 1);
    }
    if ((c[(v[l] - 1)] > maxC))
    {
      maxC = c[(v[l] - 1)];
      maxI = (v[l] - 1);
    }
    if (((((r - l) + 1) - maxC) > k))
    {
      if (((v[l] - 1) == maxI))
      {
        maxC -= 1;
      }
      c[(v[l] - 1)] -= 1;
      l += 1;
    } else if ((r == (n - 1)))
    {
      break;
    } else
    {
      r += 1;
      c[(v[r] - 1)] += 1;
      if ((c[(v[r] - 1)] > maxC))
      {
        maxC = c[(v[r] - 1)];
        maxI = (v[r] - 1);
      }
      if (((((r - l) + 1) - maxC) <= k))
      {
        res = max(res, maxC);
      }
    }
  }
  write(res);
  return 0;
}
