// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n);
  read(k);
  var arr = cpp_array(k);
  {
    var i = 0;
    while ((i < k))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var minrem = (n % arr[0]);
  var number = 0;
  var pos = 0;
  {
    var i = 0;
    while ((i < k))
    {
      var rem = (n % arr[i]);
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
