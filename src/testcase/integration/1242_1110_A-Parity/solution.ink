// Translated from solution.cpp.

func power(b: dynamic, k: dynamic) -> dynamic
{
  var product: dynamic = 1;
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

func main() -> dynamic
{
  var b: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  read(b, k);
  var arr: dynamic = cpp_array(k);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(arr[i]);
      i += 1;
    }
  }
  m = (k - 1);
  {
    var i: dynamic = 0;
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
