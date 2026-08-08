// Translated from solution.cpp.

func sortinrev(a: dynamic, b: dynamic)
{
  return ((a.first > b.first));
}

func main()
{
  var n: dynamic;
  read(n);
  var ar = cpp_array(n);
  {
    var i = 0;
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
  var pos = 0;
  var pre = cpp_array(n);
  var suff = cpp_array(n);
  pre[0] = (~ar[0]);
  {
    var i = 1;
    while ((i < n))
    {
      pre[i] = (((~ar[i]) & pre[(i - 1)]));
      i += 1;
    }
  }
  suff[(n - 1)] = (~ar[(n - 1)]);
  {
    var i = (n - 2);
    while ((i >= 0))
    {
      suff[i] = (((~ar[i]) & suff[(i + 1)]));
      i -= 1;
    }
  }
  var maxi = (ar[0] & suff[1]);
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      var val = (((pre[(i - 1)] & suff[(i + 1)])) & ar[i]);
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
    var i = 0;
    while ((i < n))
    {
      write(ar[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
