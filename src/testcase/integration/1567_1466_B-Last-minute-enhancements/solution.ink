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
        arr1[i] = (arr[i] + 1);
        i += 1;
      }
    }
    var st: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((st.find(arr[i]) == st.end()))
        {
          st.insert(arr[i]);
        } else
        {
          st.insert(arr1[i]);
        }
        i += 1;
      }
    }
    write(st.size(), "\n");
  }
}
