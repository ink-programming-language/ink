// Translated from solution.cpp.

func sortinrev(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.first > b.first));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ar: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(ar[i]);
      i += 1;
    }
  }
  if ((n == 1))
  {
    write(ar[0]);
    return 0;
  }
  var pos: dynamic = 0;
  var pre: dynamic = cpp_array(n);
  var suff: dynamic = cpp_array(n);
  pre[0] = (~ar[0]);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      pre[i] = (((~ar[i]) & pre[(i - 1)]));
      i += 1;
    }
  }
  suff[(n - 1)] = (~ar[(n - 1)]);
  {
    var i: dynamic = (n - 2);
    while ((i >= 0))
    {
      suff[i] = (((~ar[i]) & suff[(i + 1)]));
      i -= 1;
    }
  }
  var maxi: dynamic = (ar[0] & suff[1]);
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      var val: dynamic = (((pre[(i - 1)] & suff[(i + 1)])) & ar[i]);
      if ((val >= maxi))
      {
        maxi = val;
        pos = i;
      }
      i += 1;
    }
  }
  write("\n");
  if ((((pre[(n - 2)] & ar[(n - 1)])) > maxi))
  {
    pos = (n - 1);
  }
  swap(ar[0], ar[pos]);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ar[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
