// Translated from solution.cpp.

var a = cpp_array(110);

func main()
{
  var n: dynamic;
  var mini = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((a[mini] > a[i]))
      {
        mini = i;
      }
      i += 1;
    }
  }
  swap(a[mini], a[(n - 1)]);
  var maxi = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[maxi] < a[i]))
      {
        maxi = i;
      }
      i += 1;
    }
  }
  swap(a[0], a[maxi]);
  sort((a + 1), ((a + n) - 1));
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
