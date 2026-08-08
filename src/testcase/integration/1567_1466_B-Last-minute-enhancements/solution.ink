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
        arr1[i] = (arr[i] + 1);
        i += 1;
      }
    }
    var st: dynamic;
    {
      var i = 0;
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
