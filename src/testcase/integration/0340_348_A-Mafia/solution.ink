// Translated from solution.cpp.

class sort_map
{
  var num: dynamic;
  var val: dynamic;
}

func myfunc(a: dynamic, b: dynamic)
{
  return (a > b);
}

func main()
{
  var n: dynamic;
  read(n);
  var input: dynamic;
  var mmax = 0;
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var temp: dynamic;
      read(temp);
      input.push_back(temp);
      sum += temp;
      if ((temp > mmax))
      {
        mmax = temp;
      }
      i += 1;
    }
  }
  var e = ceil((sum / ((n - 1.0))));
  if ((e < mmax))
  {
    e = mmax;
  }
  write(e);
  return 0;
}
