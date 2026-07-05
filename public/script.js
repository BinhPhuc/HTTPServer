const input = document.getElementById("input");
const output = document.getElementById("output");
const submit = document.getElementById("submit");

submit.addEventListener("click", function() {
  output.textContent = input.value;
});