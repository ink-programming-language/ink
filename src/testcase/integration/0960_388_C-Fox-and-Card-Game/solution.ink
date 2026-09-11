// Translated from solution.cpp.

var vec: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var A: dynamic = 0;
  var B: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var s: dynamic = cpp_uninitialized();
      read(s);
      {
        var j: dynamic = 0;
        while ((j < (s / 2)))
        {
          var x: dynamic = cpp_uninitialized();
          read(x);
          A += x;
          j += 1;
        }
      }
      if ((s % 2))
      {
        var x: dynamic = cpp_uninitialized();
        read(x);
        vec.push_back(x);
      }
      {
        var j: dynamic = 0;
        while ((j < (s / 2)))
        {
          var x: dynamic = cpp_uninitialized();
          read(x);
          B += x;
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(vec.begin(), vec.end());
  reverse(vec.begin(), vec.end());
  {
    var i: dynamic = 0;
    while ((i < vec.size()))
    {
      if ((i % 2))
      {
        B += vec[i];
      } else
      {
        A += vec[i];
      }
      i += 1;
    }
  }
  write(A, cpp_char(" "), B, "\n");
}
