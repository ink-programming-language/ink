// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    var maxi: dynamic = arr[(n - 1)];
    var mini: dynamic = arr[(n - 1)];
    var x: dynamic = 0;
    {
      var i: dynamic = (n - 2);
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
