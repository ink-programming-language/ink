// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var count: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a);
      count.push_back(a);
      i += 1;
    }
  }
  sort(count.begin(), count.end());
  var total: dynamic = 0;
  var previous: dynamic = (count[(n - 1)] + 1);
  {
    var i: dynamic = ((n - 1));
    while ((i >= 0))
    {
      if ((previous <= count[i]))
      {
        previous -= 1;
      } else
      {
        previous = count[i];
      }
      total += (((previous > 0)) * previous);
      i -= 1;
    }
  }
  write(total, "\n");
}
