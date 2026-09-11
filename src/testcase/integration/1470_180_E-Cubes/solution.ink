// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  var c: dynamic = cpp_construct((m + 5), 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 0;
  var res: dynamic = 1;
  c[(v[0] - 1)] += 1;
  var maxC: dynamic = 1;
  var maxI: dynamic = (v[0] - 1);
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
