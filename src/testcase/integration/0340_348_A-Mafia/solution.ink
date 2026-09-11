// Translated from solution.cpp.

class sort_map
{
  var num: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
}

func myfunc(a: dynamic, b: dynamic) -> dynamic
{
  return (a > b);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var input: dynamic = cpp_uninitialized();
  var mmax: dynamic = 0;
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var temp: dynamic = cpp_uninitialized();
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
  var e: dynamic = ceil((sum / ((n - 1.0))));
  if ((e < mmax))
  {
    e = mmax;
  }
  write(e);
  return 0;
}
