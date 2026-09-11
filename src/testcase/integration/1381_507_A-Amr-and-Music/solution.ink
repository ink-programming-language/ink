// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_array(n);
  var ind: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var count: dynamic = 0;
  while ((k >= 0))
  {
    var m: dynamic = 111;
    var i: dynamic = cpp_uninitialized();
    var ans: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < n))
      {
        if ((a[i] < m))
        {
          m = a[i];
          ans = i;
        }
        i += 1;
      }
    }
    if ((m == 111))
    {
      break;
    }
    k -= m;
    a[ans] = 111;
    if ((k >= 0))
    {
      count += 1;
      ind.push_back(ans);
    }
  }
  write(count, "\n");
  for (var x: dynamic in ind)
  {
    write((x + 1), " ");
  }
  return 0;
}
