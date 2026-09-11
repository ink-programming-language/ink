// Translated from solution.cpp.

var SIZE: dynamic = 101;

var WORD_MAX: dynamic = 1001;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var S: dynamic = cpp_array(WORD_MAX);

var si: dynamic = cpp_uninitialized();

var T: dynamic = cpp_array(SIZE, SIZE);

var pi: dynamic = cpp_uninitialized();

var ti: dynamic = cpp_uninitialized();

class Solution
{
  func init() -> dynamic
  {
    }
  func get_number() -> dynamic
  {
      var line: dynamic = cpp_uninitialized();
      getline(cin, line);
      var res: dynamic = cpp_uninitialized();
      (iss >> res);
      return res;
    }
  func input() -> dynamic
  {
      n = get_number();
      if ((n == 0))
      {
        return false;
      }
      m = get_number();
      w = get_number();
      p = get_number();
      var line: dynamic = cpp_uninitialized();
      si = 0;
      while (getline(cin, line))
      {
        if ((line == "?"))
        {
          break;
        }
        S[cpp_update(si, "++")] = line;
      }
      return true;
    }
  func add_item(s: dynamic) -> dynamic
  {
      var len: dynamic = s.size();
      if ((len > w))
      {
        {
          var i: dynamic = 0;
          while ((i < len))
          {
            var t: dynamic = s.substr(i, w);
            add_item(t);
            i += w;
          }
        }
      } else
      {
        var r: dynamic = (ti % n);
        var c: dynamic = (ti / n);
        {
          var i: dynamic = 0;
          while ((i < len))
          {
            T[pi][r][((i + (w * c)) + (p * c))] = s[i];
            i += 1;
          }
        }
        ti += 1;
        if ((ti >= (n * m)))
        {
          ti = 0;
          pi += 1;
        }
      }
    }
  func solve() -> dynamic
  {
      pi = 0;
      ti = 0;
      {
        var i: dynamic = 0;
        while ((i < SIZE))
        {
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              T[i][j] = string_cpp(((m * w) + (p * ((m - 1)))), cpp_char("."));
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < si))
        {
          var word: dynamic = S[i];
          add_item(word);
          i += 1;
        }
      }
    }
  func output() -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i <= pi))
        {
          if (((i == pi) && (ti == 0)))
          {
            i += 1;
            continue;
          }
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              write(T[i][j], "\n");
              j += 1;
            }
          }
          write("#", "\n");
          i += 1;
        }
      }
      write("?", "\n");
    }
  func run() -> dynamic
  {
      while (cpp_comma(init(), input()))
      {
        solve();
        output();
      }
      return 0;
    }
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  return s.run();
}
