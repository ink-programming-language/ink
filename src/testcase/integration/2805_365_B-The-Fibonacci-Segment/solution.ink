// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var arr = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var i = 0;
  var j = 2;
  var maxi = 1;
  var sum = (arr[0] + arr[1]);
  while ((j < n))
  {
    if ((sum == arr[j]))
    {
      maxi = max(maxi, ((j - i) + 1));
      j += 1;
    } else
    {
      i = (j - 1);
      j += 1;
    }
    sum = (arr[(j - 2)] + arr[(j - 1)]);
  }
  if ((maxi >= 2))
  {
    write(maxi);
  } else
  {
    if ((arr[1] == arr[0]))
    {
      write("2");
    } else if ((n == 1))
    {
      write("1");
    } else if ((n == 2))
    {
      write("2");
    }
  }
  return 0;
}
