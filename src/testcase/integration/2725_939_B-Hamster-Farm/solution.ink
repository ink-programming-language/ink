// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n);
  read(k);
  var arr: dynamic = cpp_array(k);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var minrem: dynamic = (n % arr[0]);
  var number: dynamic = 0;
  var pos: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var rem: dynamic = (n % arr[i]);
      if ((minrem >= rem))
      {
        number = (n / arr[i]);
        minrem = rem;
        pos = i;
      }
      i += 1;
    }
  }
  write(((pos + 1)), " ", number);
}
