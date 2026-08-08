// Translated from solution.cpp.

func power(b: dynamic, k: dynamic)
{
  var product = 1;
  if ((k == 0))
  {
    product = 1;
  } else if ((k == 1))
  {
    product = b;
  } else
  {
    product = power(b, (k / 2));
    product = (product * product);
    if ((k % 2))
    {
      product = (product * b);
    }
  }
  return product;
}

func main()
{
  var b: dynamic;
  var k: dynamic;
  var sum = 0;
  var m: dynamic;
  read(b, k);
  var arr = cpp_array(k);
  {
    var i = 0;
    while ((i < k))
    {
      read(arr[i]);
      i += 1;
    }
  }
  m = (k - 1);
  {
    var i = 0;
    while ((i < k))
    {
      sum = (sum + (arr[i] * (power(b, m))));
      if ((m == 0))
      {
        m = 0;
      } else
      {
        m = (m - 1);
      }
      i += 1;
    }
  }
  if (((sum % 2) == 0))
  {
    write("even");
  } else
  {
    write("odd");
  }
  return 0;
}
