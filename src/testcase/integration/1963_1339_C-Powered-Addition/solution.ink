// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    var maxi = arr[(n - 1)];
    var mini = arr[(n - 1)];
    var x = 0;
    {
      var i = (n - 2);
      while ((i >= 0))
      {
        x = max(cpp_cast(x), cpp_cast(ceil((log(cpp_cast((((arr[i] - mini) + 1)))) / log(2.0)))));
        mini = min(mini, arr[i]);
        i -= 1;
      }
    }
    write(x, "\n");
  }
}
