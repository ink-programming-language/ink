// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var input: dynamic = cpp_uninitialized();
  var a1: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(input);
      if ((i == 0))
      {
        b.push_back(input);
        a1 = b[i];
        temp = b[i];
        a.push_back(temp);
        write(a[i], " ");
      } else
      {
        b.push_back(input);
        a1 = (b[i] + temp);
        a.push_back(a1);
        if ((temp < a.back()))
        {
          temp = a.back();
        }
        write(a[i], " ");
      }
      i += 1;
    }
  }
  return 0;
}
